#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <assert.h>
#include <memory.h>
#include "./util.h"
#include "./pressureField.h"
#include "./phantom.h"

// using namespace std;
using std::cout;
using std::endl;

/*!  The array class keeps track of the single element geometry (height and width), their spacing,
 * the element count, and the assumed speed of sound.  It also holds the transducer F-number,
 * arrays for keeping track of the transmit and receive phase for beamforming.
 */
array::array(singleGeom g, double s, int eCnt, double speed):
    spacing(s),
    eleCnt(eCnt),
    assumedSoundSpeed(speed) {
    int i;
    // default value for F num = 2
    trnsFnum = -2;
    recvFnum = -2;
    if (trnsFnum < 0)
        cout << "trnsFnum<0, so using all elements!" << endl;
    if (recvFnum < 0)
        cout << "recvFnum<0, so using all elements!" << endl;

    geom = g;

    assert(eleCnt > 0);
    transPhase = new cplx[eleCnt];
    assert(transPhase != NULL);
    recPhase = new cplx[eleCnt];
    assert(recPhase != NULL);

    // initialize lateral focus to infinity
    for (i=0; i < eleCnt; i++)
        transPhase[i] = recPhase[i] = cplxOne;

    cplx elevTransPhase, elevRecPhase;
    elevTransPhase = elevRecPhase = cplxOne;
}


/*!setup transmit focus
 */
void array::setTransFocus(double focus, double F, double freq) {
    setFocus(transPhase, focus, F, freq);
}

/*!setup receive focus
 */
void array::setRecFocus(double focus, double F, double freq) {
    setFocus(recPhase, focus, F, freq);
}


/*!  Set up either transmit or receive phase on an array
 *   If an element is off, for a given aperture the phase factor
 *   will be equal to a complex zero, canceling that contribution.
 */
void array::setFocus(cplx* phase, double focus, double F, double freq) {
    cplx iOmega = imUnit*(2*M_PI*freq);
    double timeDelay;

    // to focus to infinity
    if (focus <= 0) {
        for (int i = 0; i < eleCnt; i++) {
            timeDelay = 0;
            phase[i] = exp(iOmega*timeDelay);
        }
        return;
    }

    int eleActive = static_cast<int>(focus/(F*spacing));

    // make sure the element number is even, and that the number of
    // txdcer elements is not exceeded
    if (eleActive%2 == 1) eleActive++;

    // get the number of active elements according to the F number,
    // if F<0 activate all
    if (eleActive < 1 || eleActive > eleCnt)
        eleActive = eleCnt;


    int firstElement = (eleCnt - eleActive)/2;
    int lastElement = firstElement + eleActive;

    int i;
    for (i=0; i < firstElement; i++)
        phase[i] = cplxZero;

    // setup lateral phase factor
    for (i=firstElement; i < lastElement; i++) {
        double xLoc = (i-(eleCnt-1)/2.)*spacing;
        double timeDelay = (focus - sqrt((xLoc*xLoc) + focus*focus)) /
                                                           assumedSoundSpeed;
        phase[i] = exp(iOmega*timeDelay);
    }

    for (i=lastElement; i < eleCnt; i++)
        phase[i] = cplxZero;
}

void array::setTrsFnum(double Fnum) {
    trnsFnum = Fnum;
}

void array::setRecFnum(double Fnum) {
    recvFnum = Fnum;
}

/*!Clear the arrays holding transmit and receive phase
 */
array::~array() {
    delete[] transPhase;
    delete[] recPhase;
}


/*!FieldBuffer creates arrays that are of a sufficient size to hold information
 * about the pressure field. The constructor only creates space in memory,
 * separate functions calculate the field
 */
fieldBuffer::fieldBuffer(double f,
                         double sz,
                         const vector& sp,
                         phantom* ph,
                         double speed,
                         array* inputTrans,
                         double gap) : transFocus(f),
                                       step(sp),
                                       K(cplxZero),
                                       target(ph),
                                       assumedSoundSpeed(speed),
                                       transducer(inputTrans),
                                       phantomGap(gap)
{
    fres = new fresnelInt;
    myVector temp = target->getPhanSize();

    // ensure a 1 mmm gap between phantom and transducer
    size.z = temp.z + 2*gap;
    size.y = transducer->geom.length;
    size.x = sz;  // the beam width

    // Half the phantom size
    center.x = temp.x/2;
    center.y = temp.y/2;
    center.z = size.z/2;


    // x is lateral, y is elevational, z is axial direction
    cout << "Creating Field Buffer" <<endl;
    assert((size.x > 0) && (size.y > 0) && (size.z > 0));
    assert((step.y > 0) && (step.y > 0) && (step.z > 0));
    assert(center.z > 0);

    // denseFactor gives calculated points per element
    denseFactor = 2;
    step.x = transducer->spacing/denseFactor;

    xLen = static_cast<int>(size.x/step.x) + 1;
    yLen = static_cast<int>(size.y/step.y) + 1;
    zLen = static_cast<int>(size.z/step.z) + 1;


    // ensure an odd number of points in each array
    if (xLen%2 == 0) xLen++;
    if (yLen%2 == 0) yLen++;
    if (zLen%2 == 0) zLen++;

    xLenExtra = xLen + denseFactor*(transducer->eleCnt-1);


    // allocate space
    singleRowTransField = new cplx[xLenExtra];
    assert(singleRowTransField != NULL);

    singleRowRecField = new cplx[xLenExtra];
    assert(singleRowRecField != NULL);

    arrayPlaneSize = zLen*(yLen+1)/2;

    arrayField = new cplx[(xLen)*arrayPlaneSize];
    assert(arrayField != NULL);
}

// destructor
fieldBuffer::~fieldBuffer() {
    delete[] singleRowTransField;
    delete[] singleRowRecField;
    delete[] arrayField;
}

/*!Calculate pressure field from a single rectangular element by accurate approximation
 */
cplx fieldBuffer::getSingleElementField(const vector& fieldPoint,
                                        const singleGeom& geom,
                                        const cplx& K) {
    cplx integral;
    double r = mod(fieldPoint);
    if (r < 2E-10)
        r = 2E-10;
    cplx imK = imUnit*K;
    double k = K.real();

    // will eventually include mechanical focus in elevational direction
    double beta = 1/(2*r) - 0;
    double yOverTwoRBeta = fieldPoint.y/(2*r*beta);
    double a = geom.width;
    double b = geom.length;

    // First evaluate sinc terms
    double argX = (k*fieldPoint.x*a)/(2*M_PI*r);
    double argY = (k*fieldPoint.y*b)/(2*M_PI*r);
    double sincX, sincY;

    if (argX < 1E-8)
        sincX  = 1.;
    else
        sincX = sin(argX)/argX;

    if (argY < 1E-8)
        sincY = 1.;
    else
        sincY = sin(argY)/argY;

    // if too close, using A = zero formulation
    if (fabs(beta) < 5.e-8) {
        integral = (a*b)/r * exp(imK*r) *sincX*sincY;
    } else if (beta > 0) {
        double factor = sqrt(2*k*fabs(beta)/M_PI);
        double bOverTwo = b/2;

        double t2 = factor*(bOverTwo - yOverTwoRBeta);
        double t1 = factor*(-bOverTwo - yOverTwoRBeta);

        integral = (a/r)*sqrt(M_PI/ (2*k*beta) ) *exp(imK*r) *sincX;
        integral *= exp(-imK*fieldPoint.y*yOverTwoRBeta/(2*r) );
        integral *= fres->fastFresnel(t2) - fres->fastFresnel(t1);

    } else {    // case beta < 0
        double factor = sqrt(2*k*fabs(beta)/M_PI);
        double bOverTwo = b/2;
        double t2 = factor*(bOverTwo - yOverTwoRBeta);
        double t1 = factor*(-bOverTwo - yOverTwoRBeta);

        integral = (a/r)*sqrt(-M_PI/ (2*k*beta) ) *exp(imK*r) *sincX;
        integral *= exp(-imK*fieldPoint.y*yOverTwoRBeta/(2*r) );

        // To deal with minus sign in argument of complex exponential just
        // take complex conjugate
        integral *= conj(fres->fastFresnel(t2) - fres->fastFresnel(t1) );
    }

    return integral;
}

/*!Calculate the field seen by the transducer at each location.
 * This field is the product of incident and reflected sound at each location
 */
void fieldBuffer::calculateBufferField(double freq) {
    // setup frequency
    K = 2*M_PI*freq/target->soundSpeed() + imUnit*target->attenuation(freq);
    assert(K.real() != 0);

    vector loc;

    singleGeom perfectTrans, perfectRec;
    perfectRec = transducer->geom;
    perfectTrans = transducer->geom;

    // setup single lateral transmit/receive focus
    transducer->setTransFocus(transFocus, transducer->trsFnum(), freq);

    // loop through the depth
    for (int zIndex=-(zLen-1)/2; zIndex <=(zLen-1)/2; zIndex++) {
        // get the z coordinate, set dynamic receive focus
        loc.z = zIndex*step.z + center.z;
        transducer->setRecFocus(loc.z*assumedSoundSpeed/target->soundSpeed(),
                                transducer->recFnum(),
                                freq);

        // loop through y direction
        for (int yIndex=-(yLen-1)/2; yIndex <=0; yIndex++) {
            // get z cord
            loc.y = yIndex*step.y;

            // loop through half of the x direction
            for (int xIndex=-(xLenExtra-1)/2; xIndex <=0; xIndex++) {
                // get x cord
                loc.x = xIndex*(step.x);

                cplx tSum = cplxZero;
                cplx rSum = cplxZero;

                tSum += getSingleElementField(loc, perfectTrans, K);
                rSum += getSingleElementField(loc, perfectRec, K);

                // get the field by single element
                singleRowTransField[xIndex+(xLenExtra-1)/2] = tSum;
                singleRowRecField[xIndex+(xLenExtra-1)/2] = rSum;
            }


            // get another half of x by symmetry
            for (int xIndex=(xLenExtra+1)/2; xIndex < xLenExtra; xIndex++) {
                singleRowTransField[xIndex] = singleRowTransField[xLenExtra-1-xIndex];
                singleRowRecField[xIndex] = singleRowRecField[xLenExtra-1-xIndex];
            }


            int tempXIndex1, tempXIndex2, index1, index2;
            // get array field by superpose all the elements
            for (int i=0; i < (xLen+1)/2; i++) {
                cplx a0Transmit = cplxZero;
                cplx a0Receive = cplxZero;

                // sum over all the elements
                for (int j=0; j < transducer->eleCnt; j++) {
                    a0Transmit += singleRowTransField[i+j*denseFactor] *
                                                     transducer->transPhase[j];
                    a0Receive += singleRowRecField[i+j*denseFactor] *
                                                     transducer->recPhase[j];
                }

                // save the result to the buffer
                tempXIndex1 = (xLen-1)/2-i;
                index1 = (tempXIndex1 + (xLen-1)/2)*arrayPlaneSize
                + (yIndex + (yLen-1)/2)*zLen
                + (zIndex + (zLen-1)/2);
                arrayField[index1] = a0Transmit*a0Receive;

                tempXIndex2 = i-(xLen-1)/2;
                index2 = (tempXIndex2 + (xLen-1)/2)*arrayPlaneSize
                + (yIndex + (yLen-1)/2)*zLen
                + (zIndex + (zLen-1)/2);
                arrayField[index2] = arrayField[index1];
            }
        }
    }
}


/*!Estimate the field seen by the transducer at location loc by using nearest neighbor
 * interpolation from the value in the buffer.
 *
 * The phase calculation is exact, however.  This is the purpose of the
 * phase term.
 */
cplx fieldBuffer::bufferField(const vector& loc) {
    int xIndex = static_cast<int>(floor(loc.x/step.x + .5));
    int yIndex = static_cast<int>(floor(loc.y/step.y + .5));

    // zIndex will run from roughly -zLen/2 to zLen/2
    int zIndex = static_cast<int>(floor((loc.z-center.z)/step.z + .5));

    if (yIndex > 0) yIndex = -yIndex;

    double zc = center.z + zIndex*step.z;

    cplx temp;
    int index = (xIndex + (xLen-1)/2)*arrayPlaneSize
            + (yIndex + (yLen-1)/2)*zLen
            + (zIndex + (zLen-1)/2);


    // the phase term should be the difference in r rather
    // than z if the beam have angle
    return arrayField[index]*exp(2.*(loc.z-zc)*imUnit*K);
}




/*! Given a position on the phantom. 0<x<phantom size
 * 0<y<phantom size
 * 0<z<phantom size
 * Work out the position relative to the transducer
 * This includes adding in the gap between the transducer
 * and the phantom, also the fact that the beam is only calculated
 * once for all beams formed by a linear array transducer.
 *
 *  pressure coordinates run from: -beamSize/2<x<beamSize/2
 *  -beamSize/2<y<beamSize/2
 *  0<z<beamSize
 */

vector fieldBuffer::phantomCoordinateToPressureCoordinate(
                                                const scatterer& inVector,
                                                int beamLine) {
vector outVector;
// x-coordinate
double leftEndX = beamLine*transducer->spacing;
double distanceFromLeftEndOfBeam = inVector.x - leftEndX;
outVector.x = -fieldBuffer::size.x/2 + distanceFromLeftEndOfBeam;
// y-coordinate
outVector.y = inVector.y - center.y;
// z-coordinate
outVector.z = inVector.z+ fieldBuffer::phantomGap;

return outVector;
}
