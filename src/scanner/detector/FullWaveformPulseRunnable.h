#pragma once

#include <util/HeliosException.h>
#include "AbstractDetector.h"
#include "AbstractPulseRunnable.h"
#include "FullWaveformPulseDetector.h"
#include "RaySceneIntersection.h"
#include "ScenePart.h"
#include <noise/RandomnessGenerator.h>
#include <scanner/ScanningPulseProcess.h>

#include <LadLut.h>

#include <vector>

/**
 * @brief Concrete implementation of abstract pulse runnable to compute full
 * waveform pulses
 *
 * @see AbstractPulseRunnable
 */
class FullWaveformPulseRunnable : public AbstractPulseRunnable {
public:
    // ***  CONSTANTS  *** //
    // ******************* //
    /**
     * @brief Decimal precision constant for FullWaveformPulseRunnable
     * computations
     */
    static const double eps;
private:
    // ***  ATTRIBUTES  *** //
    // ******************** //
    /**
     * @brief Full waveform pulse detector used to handle pulse computation
     */
	std::shared_ptr<FullWaveformPulseDetector> fwDetector = nullptr;

public:

    // ***  CONSTRUCTION / DESTRUCTION  *** //
    // ************************************ //
    /**
     * @brief Base constructor for full waveform pulse runnable
     * @see AbstractPulseRunnable::AbstractPulseRunnable(
     *  std::shared_ptr<AbstractDetector>, SimulatedPulse const &)
     * @see SimulatedPulse
     */
    FullWaveformPulseRunnable(
        std::shared_ptr<Scanner> scanner,
        SimulatedPulse const &pulse
    ) :
        AbstractPulseRunnable(scanner, pulse)
	{}

	virtual ~FullWaveformPulseRunnable(){}

private:
    // ***  OPERATOR METHODS  *** //
    // ************************** //
    /**
     * @brief Initialize pending attributes of the full waveform pulse runnable
     *  before doing further computations. This implies calling the
     *  AbstractPulseRunnable::initialize method
     *
     * NOTE that this method alleviates the burden of the sequential thread by
     *  supporting deferred initialization whenever possible.
     * @see AbstractPulseRunnable::initialize
     */
    void initialize() override;
    /**
     * @brief Perform ray casting to find intersections
     * @param[in] tMinMax Minimum and maximum time to intersection with respect
     *  to the axis aligned bounding box that bounds the scene
     * @param[out] reflections Where reflections must be stored when a hit is
     *  registered
     * @param[out] intersects Where intersections must be stored when a hit is
     *  registered
     * @see FullWaveformPulseRunnable::handleSubray
     */
    void computeSubrays(
        vector<double> const &tMinMax,
        NoiseSource<double> &intersectionHandlingNoiseSource,
        std::map<double, double> &reflections,
        vector<RaySceneIntersection> &intersects
    );
    /**
     * @brief Handle sub-rays along the circle
     * @param[in] tMinMax Minimum and maximum time to intersection with respect
     *  to the axis aligned bounding box that bounds the scene
     * @param[in] circleStep The iteration along the circle
     * @param[in] circleStep_rad Angle in radians corresponding to the
     *  iteration
     * @param[in] r1 Sub-beam rotation into divergence step
     * @param[in] divergenceAngle Subray divergence angle in radians
     * @see FullWaveformPulseRunnable::computeSubrays
     */
    void handleSubray(
        vector<double> const &tMinMax,
        int const circleStep,
        double const circleStep_rad,
        Rotation &r1,
        double const divergenceAngle,
        NoiseSource<double> &intersectionHandlingNoiseSource,
        std::map<double, double> &reflections,
        vector<RaySceneIntersection> &intersects
    );
    /**
     * @brief Digest intersections found through ray casting
     * @param[in] beamDir Beam normalized director vector
     * @param[out] reflections Where reflections must be stored when a hit is
     *  registered
     * @param[out] intersects Where intersections must be stored when a hit is
     *  registered
     * @see FullWaveformPulseRunnable::computeSubrays
     * @see FullWaveformPulseRunnable::findMaxMinHitDistances
     * @see FullWaveformPulseRunnable::initializeFullWaveform
     * @see FullWaveformPulseRunnable::populateFullWaveform
     * @see FullWaveformPulseRunnable::digestFullWaveform
     * @see FullWaveformPulseRunnable::exportOutput
     */
    void digestIntersections(
        std::vector<std::vector<double>>& apMatrix,
        RandomnessGenerator<double> &randGen,
        RandomnessGenerator<double> &randGen2,
        glm::dvec3 &beamDir,
        std::map<double, double> &reflections,
        vector<RaySceneIntersection> &intersects
    );
    /**
     * @brief Find min and max hit distances in meters
     * @param[out] minHitDist_m Min hit distance will be stored here
     * @param[out] maxHitDist_m Max hit distance will be stored here
     * @see FullWaveformPulseRunnable::digestIntersections
     */
    void findMaxMinHitDistances(
        std::map<double, double> &reflections,
        double &minHitDist_m,
        double &maxHitDist_m
    );
    /**
     * @brief Initialize full waveform
     * While the vector is not strictly initialized in this function,
     * necessary variables are computed here.
     * @param[out] nsPerBin The size of each bin in nano seconds
     * @param[out] distanceThreshold Limit distance threshold
     * @param[out] peakIntensityIndex Index of intensity peak
     * @param[out] numFullwaveBins How many bins are necessary to discretize
     *  the full waveform
     * @return True if it is possible to initialize the full waveform,
     * false otherwise.
     * @see FullWaveformPulseRunnable::digestIntersections
     */
    bool initializeFullWaveform(
        double const minHitDist_m,
        double const maxHitDist_m,
        double &minHitTime_ns,
        double &maxHitTime_ns,
        double &nsPerBin,
        double &distanceThreshold,
        int &peakIntensityIndex,
        int &numFullwaveBins
    );
    /**
     * @brief Populate a previously initialized full waveform vector
     * @param[out] fullwave Full waveform vector to be populated
     * @see FullWaveformPulseRunnable::digestIntersections
     * @see FullWaveformPulseRunnable::initializeFullWaveform
     */
    void populateFullWaveform(
        std::map<double, double> const &reflections,
        std::vector<double> &fullwave,
        double const distanceThreshold,
        double const minHitTime_ns,
        double const nsPerBin,
        int const peakIntensityIndex
    );
    /**
     * @brief Digest a previously populated full waveform vector,
     * generating measurements
     * @param[out] pointsMeasurement Where generated measurements will be
     * stored
     * @param[out] numReturns Number of returns will be stored here
     * @param fullwave Full waveform vector to be digested
     * @see FullWaveformPulseRunnable::digestIntersections
     * @see FullWaveformPulseRunnable::populateFullWaveform
     * @see Measurement
     */
    void digestFullWaveform(
        std::vector<Measurement> &pointsMeasurement,
        int &numReturns,
        std::vector<std::vector<double>>& apMatrix,
        std::vector<double> const &fullwave,
        vector<RaySceneIntersection> const &intersects,
        glm::dvec3 const &beamDir,
        double const nsPerBin,
        int const numFullwaveBins,
        int const peakIntensityIndex,
        double const minHitTime_ns
    );
    /**
     * @brief Export measurements and full waveform data
     * @param[in] fullwave Full waveform data to export
     * @param[in] pointsMeasurement Point cloud data to export
     * @see FullWaveformPulseRunnable::digestIntersections
     */
    void exportOutput(
        std::vector<double> &fullwave,
        int const numReturns,
        std::vector<Measurement> &pointsMeasurement,
        glm::dvec3 const &beamDir,
        double const minHitTime_ns,
        double const maxHitTime_ns,
        RandomnessGenerator<double> &randGen,
        RandomnessGenerator<double> &randGen2
    );

    // ***  ASSISTANCE METHODS  *** //
    // **************************** //
    /**
     * @brief Find the intersection between the scene and given ray, if any
     * @param[in] tMinMax Minimum and maximum time to intersection with respect
     *  to the axis aligned bounding box that bounds the scene
     * @param o The ray origin
     * @param v The ray director vector
     * @return Intersection between the scene and given ray
     */
    virtual shared_ptr<RaySceneIntersection> findIntersection(
        vector<double> const &tMinMax,
        glm::dvec3 const &o,
        glm::dvec3 const &v
    ) const;
    /**
     * @brief Detect full waveform peaks
     */
    bool detectPeak(
        int const i,
        int const win_size,
        vector<double> const &fullwave
    );

	/**
	 * @brief Capture full wave
	 * @param fullwave Full wave vector
	 * @param fullwaveIndex Full wave index
	 * @param min_time Minimum hit time (nanoseconds)
	 * @param max_time Maximum hit time (nanoseconds)
	 * @param beamOrigin Beam origin in absolute coordinates
	 * @param beamDir Beam director vector
	 * @param gpstime Current GPS time (nanoseconds)
	 * @param fullWaveNoise flag to specify if noise must be added to the
	 *  full wave (true) or not (false)
	 * @param rg2 Randomness generator to be used to add noise to the full wave
	 *  if requested
	 */
	void captureFullWave(
	    std::vector<double> & fullwave,
	    int const fullwaveIndex,
	    double const min_time,
	    double const max_time,
	    glm::dvec3 const &beamOrigin,
	    glm::dvec3 const &beamDir,
        double const gpstime,
	    bool const fullWaveNoise,
	    RandomnessGenerator<double> &rg2
    );

public:
	// ***  O P E R A T O R  *** //
	// ************************* //
	/**
	 * @brief Full waveform pulse runnable void functor. It is necessary
	 * due to compatibility reasons.
	 * @see AbstractPulseRunnable::operator()
	 */
	void operator()() override {
	    throw HeliosException(
	        "FullWaveformPulseRunnable operator()() must not be used"
        );
	}
	/**
	 * @brief Full waveform pulse runnable functor
	 * @param apMatrix Reference to matrix to be used to compute Marquardt
	 * fitter
	 * @param randGen A randomness generator
	 * @param randGen2 Another randomness generator
	 * @param intersectionHandlingNoiseSource Noise source to be used at
	 * intersection handling if necessary
	 * @see AbstractPulseRunnable::operator()()
	 */
	void operator() (
	    std::vector<std::vector<double>>& apMatrix,
	    RandomnessGenerator<double> &randGen,
        RandomnessGenerator<double> &randGen2,
        NoiseSource<double> &intersectionHandlingNoiseSource
    ) override;
};