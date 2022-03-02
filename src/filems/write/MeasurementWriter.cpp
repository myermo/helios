#include <filems/write/MeasurementWriter.h>
#include <util/HeliosException.h>

using namespace helios::filems;

using std::stringstream;

// ***   M E T H O D S   *** //
// ************************* //
void MeasurementWriter::writeMeasurement(Measurement & m){
    // Check there is a sync file writer
    if(sfw == nullptr){
        throw HeliosException(
            "MeasurementWriter::writeMeasurement failed because there was no "
            "SyncFileWriter (sfw) available"
        );

    }
    // Check there is a scanner
    else if(scanner == nullptr){
        throw HeliosException(
            "MeasurementWriter::writeMeasurement failed because there was no "
            "Scanner to associate measurements with"
        );
    }

    // Write measured point to output file
    sfw->write(m, scanner->platform->scene->getShift());
}

void MeasurementWriter::writeMeasurements(list<Measurement*> & measurements){
    // Check that there is an available writer
    if(sfw == nullptr){
        throw HeliosException(
            "MeasurementWriter::writeMeasurements failed because there was no "
            "SyncFileWriter (sfw) available"
        );
    }
    // Check there is a scanner
    else if(scanner == nullptr){
        throw HeliosException(
            "MeasurementWriter::writeMeasurements failed because there was no "
            "Scanner to associate measurements with"
        );
    }

    // Write measured point to output file
    for (const Measurement* m : measurements) {
        sfw->write(*m, scanner->platform->scene->getShift());
    }
}

WriterType MeasurementWriter::chooseWriterType() const {
    // Get the type of writer to be created
    WriterType wt;
    if (lasOutput) wt = las10 ? las10Type : las14Type;
    else if (las10) wt = las10Type;
    else if (zipOutput) wt = zipType;
    else wt = simpleType;

    return wt;
}

void MeasurementWriter::finish(){
    // If there is no sync file writer to finish, throw an exception
    if(sfw == nullptr){
        throw HeliosException(
            "MeasurementWriter::finish had not sync file writer (sfw) to "
            "finish"
        );
    }
    // Finish sync file writer, if any
    sfw->finish();
}


// ***  GETTERs and SETTERs  *** //
// ***************************** //
// ATTENTION: This method needs to be synchronized since multiple threads are
// writing to the output file!
void MeasurementWriter::setOutputFilePath(string path, bool lastLegInStrip){
    outputFilePath = path;
    logging::WARN("outputFilePath=" + path);
    try {
        fs::create_directories(outputFilePath.parent_path());
        WriterType wt = chooseWriterType();

        // Create the Writer
        if(!fs::exists(path)){
            sfw = SyncFileWriterFactory::makeWriter(
                wt,
                path,                                   // Output path
                zipOutput,                              // Zip flag
                lasScale,                               // Scale factor
                scanner->platform->scene->getShift(),   // Offset
                0.0,                                    // Min intensity
                1000000.0                               // Delta intensity
            );
            writers[path] = sfw;
        }
        else{ // Consider existing writer
            sfw = writers[path];
        }

        // Remove writer from writers hashmap if it is the last leg in strip
        // to allow the sfw destructor to be called when sfw is replaced in the
        // next leg
        if(lastLegInStrip) writers.erase(path);

    } catch (std::exception &e) {
        logging::WARN(e.what());
    }
}
