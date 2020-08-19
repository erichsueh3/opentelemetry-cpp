#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>
#include <unistd.h>
#include <typeinfo>
#include <variant>
#include <unordered_map>
#include <map>
#include <chrono>

#include "opentelemetry/exporters/prometheus/prometheus_exporter.h"
#include "opentelemetry/sdk/metrics/controller.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/ungrouped_processor.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/metrics/provider.h"

namespace metrics_api = opentelemetry::metrics;
namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace prometheus_exporter = opentelemetry::exporter::prometheus;

using namespace std;

void IntObserverConstructorCallback(metrics_api::ObserverResult<int> result){}

void ShortObserverConstructorCallback(metrics_api::ObserverResult<short> result){}

void FloatObserverConstructorCallback(metrics_api::ObserverResult<float> result){}

void DoubleObserverConstructorCallback(metrics_api::ObserverResult<double> result){}

class MetricGenerator {
    
public:
    
    static void generateData() {
        int interval = 5*1000;
        std::cerr <<"initializing components" <<std::endl;
        
        auto provider = opentelemetry::nostd::shared_ptr<metrics_api::MeterProvider>(new metrics_sdk::MeterProvider);
        opentelemetry::metrics::Provider::SetMeterProvider(provider);
        
        // 1. Initialize exporter
        std::string addr = "localhost:8080";
        std::unique_ptr<metrics_sdk::MetricsExporter> e = std::unique_ptr<metrics_sdk::MetricsExporter>(new prometheus_exporter::PrometheusExporter(addr));
        
        // 2. Initialize processor
        std::shared_ptr<metrics_sdk::MetricsProcessor> p = std::shared_ptr<metrics_sdk::MetricsProcessor>(new metrics_sdk::UngroupedMetricsProcessor(false));
        
        // 3. Initialize meter
        opentelemetry::nostd::shared_ptr<metrics_api::Meter> m = provider->GetMeter("Test");
        
        // 4. Initialize controller
        metrics_sdk::PushController c(m, std::move(e),p, interval/1000);
        c.start();
        
        // 5. use these to instrument some work
        std::cerr <<"Simulating work" <<std::endl;
        auto start = std::chrono::steady_clock::now();
        doSomeSimulatedWork(m);
        auto end = std::chrono::steady_clock::now();
        std::cerr <<"Simulation complete after: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() <<" milliseconds" <<std::endl;
        
        
        // 6. shutdown metric collector
        c.stop();
        std::cerr <<"controller shutdown" <<std::endl;
        
    }
    
private:
    //    static void doSomeSimulatedWork(metrics::api::Meter &m){
    static map<string,string> str2map(string alpha){
        map<string,string> ret;
        while (alpha.find(',') != string::npos){
            string key = alpha.substr(0,alpha.find(','));
            alpha = alpha.substr(alpha.find(',')+2);
            string value = alpha.substr(0,alpha.find(','));
            alpha = alpha.substr(alpha.find(',')+2);
            ret[key] = value;
        }
        return ret;
    }
    
    static void doSomeSimulatedWork(opentelemetry::nostd::shared_ptr<metrics_api::Meter> m){
        ifstream goldenData;
        goldenData.open("exporters/prometheus/test/PrometheusDataFirst.csv");
        
        auto ictr= m->NewIntCounter("ictr","none", "none", true);
        auto iudctr= m->NewIntUpDownCounter("iudctr","none", "none", true);
        auto ivrec= m->NewIntValueRecorder("ivrec","none", "none", true);
        auto isobs= m->NewIntSumObserver("isobs","none", "none", true, &IntObserverConstructorCallback);
        auto iudobs= m->NewIntUpDownSumObserver("iudobs","none", "none", true, &IntObserverConstructorCallback);
        auto ivobs= m->NewIntValueObserver("ivobs","none", "none", true, &IntObserverConstructorCallback);
        
        auto sctr= m->NewShortCounter("sctr","none", "none", true);
        auto sudctr= m->NewShortUpDownCounter("sudctr","none", "none", true);
        auto svrec= m->NewShortValueRecorder("svrec","none", "none", true);
        auto ssobs= m->NewShortSumObserver("ssobs","none", "none", true, &ShortObserverConstructorCallback);
        auto sudobs= m->NewShortUpDownSumObserver("sudobs","none", "none", true, &ShortObserverConstructorCallback);
        auto svobs= m->NewShortValueObserver("svobs","none", "none", true, &ShortObserverConstructorCallback);
        
        auto fctr= m->NewFloatCounter("fctr","none", "none", true);
        auto fudctr= m->NewFloatUpDownCounter("fudctr","none", "none", true);
        auto fvrec= m->NewFloatValueRecorder("fvrec","none", "none", true);
        auto fsobs= m->NewFloatSumObserver("fsobs","none", "none", true, &FloatObserverConstructorCallback);
        auto fudobs= m->NewFloatUpDownSumObserver("fudobs","none", "none", true, &FloatObserverConstructorCallback);
        auto fvobs= m->NewFloatValueObserver("fvobs","none", "none", true, &FloatObserverConstructorCallback);
        
        auto dctr= m->NewDoubleCounter("dctr","none", "none", true);
        auto dudctr= m->NewDoubleUpDownCounter("dudctr","none", "none", true);
        auto dvrec= m->NewDoubleValueRecorder("dvrec","none", "none", true);
        auto dsobs= m->NewDoubleSumObserver("dsobs","none", "none", true, &DoubleObserverConstructorCallback);
        auto dudobs= m->NewDoubleUpDownSumObserver("dudobs","none", "none", true, &DoubleObserverConstructorCallback);
        auto dvobs= m->NewDoubleValueObserver("dvobs","none", "none", true, &DoubleObserverConstructorCallback);
        
        
        // EXCEPTIONS: Can occur when an appropriate update value is passed to an instrument,
        // an invalid name, or a duplicated name
        
        std::string line;
        while (std::getline(goldenData,line)){
            std::string instrument = line.substr(0,line.find(','));
            int val = stoi(line.substr(line.find(',')+1,line.find('"')-line.find(',')-2));
            std::string labels = line.substr(line.find('"')+1, line.rfind('"')-line.find('"')-1);
            map<string,string> labelmap = str2map(labels);
            auto labelkv = opentelemetry::trace::KeyValueIterableView<decltype(labelmap)>{labelmap};

            if (instrument == "ictr"){
                ictr->add(val, labelkv);
            } else if (instrument == "iudctr"){
                iudctr->add(val, labelkv);
            } else if (instrument == "ivrec"){
                ivrec->record(val, labelkv);
            } else if (instrument == "isobs"){
                isobs->observe(val, labelkv);
            } else if (instrument == "iudobs"){
                iudobs->observe(val, labelkv);
            } else if (instrument == "ivobs"){
                ivobs->observe(val, labelkv);
            } else if (instrument == "sctr"){
                sctr->add(val, labelkv);
            } else if (instrument == "sudctr"){
                sudctr->add(val, labelkv);
            } else if (instrument == "svrec"){
                svrec->record(val, labelkv);
            } else if (instrument == "ssobs"){
                ssobs->observe(val, labelkv);
            } else if (instrument == "sudobs"){
                sudobs->observe(val, labelkv);
            } else if (instrument == "svobs"){
                svobs->observe(val, labelkv);
            } else if (instrument == "fctr"){
                fctr->add(val, labelkv);
            } else if (instrument == "fudctr"){
                fudctr->add(val, labelkv);
            } else if (instrument == "fvrec"){
                fvrec->record(val, labelkv);
            } else if (instrument == "fsobs"){
                fsobs->observe(val, labelkv);
            } else if (instrument == "fudobs"){
                fudobs->observe(val, labelkv);
            } else if (instrument == "fvobs"){
                fvobs->observe(val, labelkv);
            } else if (instrument == "dctr"){
                dctr->add(val, labelkv);
            } else if (instrument == "dudctr"){
                dudctr->add(val, labelkv);
            } else if (instrument == "dvrec"){
                dvrec->record(val, labelkv);
            } else if (instrument == "dsobs"){
                dsobs->observe(val, labelkv);
            } else if (instrument == "dudobs"){
                dudobs->observe(val, labelkv);
            } else if (instrument == "dvobs"){
                dvobs->observe(val, labelkv);
            } else {
                std::cerr <<"Bad entry" <<std::endl;
            }
            //usleep(.1*1000000);
        }
        
    }
};


int main(int argc, const char * argv[]) {
    MetricGenerator::generateData();
}
