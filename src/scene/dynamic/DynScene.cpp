#include <scene/dynamic/DynScene.h>
#include <logging.hpp>
#include <SerialIO.h>

using std::stringstream;

// ***  CONSTRUCTION / DESTRUCTION  *** //
// ************************************ //
DynScene::DynScene(DynScene &ds) : DynScene(static_cast<StaticScene&>(ds)) {
    for(shared_ptr<DynObject> dynObj : ds.dynObjs){
        dynObjs.push_back(dynObj);
        updated.push_back(true);
    }
    makeStepLoop(ds.stepLoop.getStepInterval());
    stepLoop.setCurrentStep(ds.stepLoop.getCurrentStep());
}

// ***  SIMULATION STEP  *** //
// ************************* //
bool DynScene::doSimStep(){
    if(stepLoop.doStep()) return stepLoop.retrieveOutput();
    return false;
}

bool DynScene::doStep(){
    bool updateFlag = false;
    size_t const n = numDynObjects();
    for(size_t i = 0 ; i < n ; ++i){
        updated[i] = dynObjs[i]->doStep();
        updateFlag |= updated[i];
    }
    return updateFlag;
}

void DynScene::makeStepLoop(int const stepInterval){
    this->stepLoop = NonVoidStepLoop<bool>(
        stepInterval, [&] () -> bool{return doStep();}
    );
}


// ***   READ/WRITE  *** //
// ********************* //
void DynScene::writeObject(std::string path){
    stringstream ss;
    ss << "Writing dynamic scene object to " << path << " ...";
    logging::INFO(ss.str());
    SerialIO::getInstance()->write<DynScene>(path, this);
}
DynScene * DynScene::readObject(std::string path){
    stringstream ss;
    ss << "Reading dynamic scene object from " << path << " ...";
    logging::INFO(ss.str());
    return SerialIO::getInstance()->read<DynScene>(path);
}
