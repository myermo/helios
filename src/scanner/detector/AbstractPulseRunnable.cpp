#include "AbstractPulseRunnable.h"

#define _USE_MATH_DEFINES
#include "math.h"
#include "MathConstants.h"
#include <maths/EnergyMaths.h>

#include <glm/glm.hpp>

#include "AbstractDetector.h"
#include <filems/facade/FMSFacade.h>


// Energy left after attenuation by air particles in range [0,1]
inline double AbstractPulseRunnable::calcAtmosphericFactor(
    double const targetRange
) const {
    return EnergyMaths::calcAtmosphericFactor(
        targetRange, detector->scanner->getAtmosphericExtinction(0)
    );
}

// Laser radar equation "Signature simulation..." (Carlsson et al., 2000)
double AbstractPulseRunnable::calcReceivedPower(
    double const emittedPower,
    double const targetRange,
    double const incidenceAngle,
    double const targetReflectivity,
    double const targetSpecularity,
    double const targetSpecularExponent,
    double const targetArea
) const {
	double const bdrf = targetReflectivity * EnergyMaths::phongBDRF(
	    incidenceAngle,
	    targetSpecularity,
	    targetSpecularExponent
    );
	double const sigma = EnergyMaths::calcCrossSection(
	    bdrf, targetArea, incidenceAngle
    );
    return AbstractPulseRunnable::_calcReceivedPower(
        emittedPower,
        detector->scanner->getDr2(0),
        targetRange,
        detector->scanner->getBt2(0),
        detector->scanner->getEfficiency(0),
        calcAtmosphericFactor(targetRange),
        sigma
    );
}

double AbstractPulseRunnable::calcReceivedPower(
    double const emittedPower,
    double const targetRange,
    double const sigma
) const {
    return AbstractPulseRunnable::_calcReceivedPower(
        emittedPower,
        detector->scanner->getDr2(0),
        targetRange,
        detector->scanner->getBt2(0),
        detector->scanner->getEfficiency(0),
        calcAtmosphericFactor(targetRange),
        sigma
    );
}

inline double AbstractPulseRunnable::_calcReceivedPower(
    double const Pt,
    double const Dr2,
    double const R,
    double const Bt2,
    double const etaSys,
    double const etaAtm,
    double const sigma
){
    return (Pt * Dr2) / (PI_4 * pow(R, 4) * Bt2) * etaSys * etaAtm * sigma;
}

void AbstractPulseRunnable::capturePoint(
    Measurement & m,
    RandomnessGenerator<double> &rg,
    std::vector<Measurement> *allMeasurements,
    std::mutex *allMeasurementsMutex,
    std::vector<Measurement> *cycleMeasurements,
    std::mutex *cycleMeasurementsMutex
) {
	if (!writeGround && m.classification == LasSpecification::GROUND) {
		return;
	}

	// Abort if point distance is below mininum scanner range:
	// TODO Pending : This check is already done in FullWaveformPulseRunnable
	// What is the point on repeating it?
	if (m.distance < detector->cfg_device_rangeMin_m) {
		return;
	}
	// ########## BEGIN Apply gaussian range accuracy error ###########
	applyMeasurementError(rg, m.distance, m.beamOrigin, m.beamDirection);
	// ########## END Apply gaussian range accuracy error ###########

	// Calculate final recorded point coordinates:
	// TODO Pending : Is it necessary to compute position again? Notice it is
    // known from ray intersection point at FullWaveformPulseRunnable
	m.position = m.beamOrigin + m.beamDirection * m.distance;
    if(allMeasurements != nullptr){
        std::unique_lock<std::mutex> lock(*allMeasurementsMutex);
        allMeasurements->push_back(m);
        (allMeasurements->end() - 1)->position +=
            detector->scanner->platform->scene->getShift();
    }
    if(cycleMeasurements != nullptr){
        std::unique_lock<std::mutex> lock(*cycleMeasurementsMutex);
        cycleMeasurements->push_back(m);
        (cycleMeasurements->end() - 1)->position +=
            detector->scanner->platform->scene->getShift();
    }
    detector->pcloudYielder->push(m);
}

void AbstractPulseRunnable::applyMeasurementError(
    RandomnessGenerator<double> &rg,
    double &distance,
    glm::dvec3 &beamOrigin,
    glm::dvec3 &beamDirection
){
    distance += rg.normalDistributionNext();
}

