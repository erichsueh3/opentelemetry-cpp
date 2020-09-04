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

#include "opentelemetry/sdk/metrics/record.h"
#include "opentelemetry/sdk/metrics/aggregator/aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/counter_aggregator.h"

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
        std::unique_ptr<prometheus_exporter::PrometheusExporter> e = std::unique_ptr<prometheus_exporter::PrometheusExporter>(new prometheus_exporter::PrometheusExporter(addr));
        
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
        // c.stop();
        std::cerr <<"controller shutdown" <<std::endl;
        
        while(true){
            std::cerr << "waiting for scrape from Prometheus" << std::endl;
            usleep(.1*10000000);
        }        
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
   
        // EXCEPTIONS: Can occur when an appropriate update value is passed to an instrument,
        // an invalid name, or a duplicated name
        
        std::string line;
        while (std::getline(goldenData,line)){
            std::string instrument = line.substr(0,line.find(','));
            int val = stoi(line.substr(line.find(',')+1,line.find('"')-line.find(',')-2));
            std::string ndl = line.substr(line.find('"')+1, line.rfind('"')-line.find('"')-1);
            std::string name = ndl.substr(0, ndl.find(','));
            std::string description = ndl.substr(name.size() + 2, ndl.find(',', name.size()+1)-ndl.find(',')-2);
            std::string labels = ndl.substr(name.size() + description.size()+4);

            map<string,string> labelmap = str2map(labels);
            auto labelkv = opentelemetry::trace::KeyValueIterableView<decltype(labelmap)>{labelmap};

            if (instrument == "ictr"){
                auto ictr= m->NewIntCounter(name, description, "none", true);
                ictr->add(val, labelkv);
            } else if (instrument == "iudctr"){
                auto iudctr= m->NewIntUpDownCounter(name, description, "none", true);
                iudctr->add(val, labelkv);
            } else if (instrument == "ivrec"){
                auto ivrec= m->NewIntValueRecorder(name, description, "none", true);
                ivrec->record(val, labelkv);
            } else if (instrument == "isobs"){
                auto isobs= m->NewIntSumObserver(name, description, "none", true, &IntObserverConstructorCallback);
                isobs->observe(val, labelkv);
            } else if (instrument == "iudobs"){
                auto iudobs= m->NewIntUpDownSumObserver(name, description, "none", true, &IntObserverConstructorCallback);
                iudobs->observe(val, labelkv);
            } else if (instrument == "ivobs"){
                auto ivobs= m->NewIntValueObserver(name, description, "none", true, &IntObserverConstructorCallback);
                ivobs->observe(val, labelkv);
            } else if (instrument == "sctr"){
                auto sctr= m->NewShortCounter(name, description, "none", true);
                sctr->add(val, labelkv);
            } else if (instrument == "sudctr"){
                auto sudctr= m->NewShortUpDownCounter(name, description, "none", true);
                sudctr->add(val, labelkv);
            } else if (instrument == "svrec"){
                auto svrec= m->NewShortValueRecorder(name, description, "none", true);
                svrec->record(val, labelkv);
            } else if (instrument == "ssobs"){
                auto ssobs= m->NewShortSumObserver(name, description, "none", true, &ShortObserverConstructorCallback);
                ssobs->observe(val, labelkv);
            } else if (instrument == "sudobs"){
                auto sudobs= m->NewShortUpDownSumObserver(name, description, "none", true, &ShortObserverConstructorCallback);
                sudobs->observe(val, labelkv);
            } else if (instrument == "svobs"){
                auto svobs= m->NewShortValueObserver(name, description, "none", true, &ShortObserverConstructorCallback);
                svobs->observe(val, labelkv);
            } else if (instrument == "fctr"){
                auto fctr= m->NewFloatCounter(name, description, "none", true);
                fctr->add(val, labelkv);
            } else if (instrument == "fudctr"){
                auto fudctr= m->NewFloatUpDownCounter(name, description, "none", true);
                fudctr->add(val, labelkv);
            } else if (instrument == "fvrec"){
                auto fvrec= m->NewFloatValueRecorder(name, description, "none", true);
                fvrec->record(val, labelkv);
            } else if (instrument == "fsobs"){
                auto fsobs= m->NewFloatSumObserver(name, description, "none", true, &FloatObserverConstructorCallback);
                fsobs->observe(val, labelkv);
            } else if (instrument == "fudobs"){
                auto fudobs= m->NewFloatUpDownSumObserver(name, description, "none", true, &FloatObserverConstructorCallback);
                fudobs->observe(val, labelkv);
            } else if (instrument == "fvobs"){
                auto fvobs= m->NewFloatValueObserver(name, description, "none", true, &FloatObserverConstructorCallback);
                fvobs->observe(val, labelkv);
            } else if (instrument == "dctr"){
                auto dctr= m->NewDoubleCounter(name, description, "none", true);
                dctr->add(val, labelkv);
            } else if (instrument == "dudctr"){
                auto dudctr= m->NewDoubleUpDownCounter(name, description, "none", true);
                dudctr->add(val, labelkv);
            } else if (instrument == "dvrec"){
                auto dvrec= m->NewDoubleValueRecorder(name, description, "none", true);
                dvrec->record(val, labelkv);
            } else if (instrument == "dsobs"){
                auto dsobs= m->NewDoubleSumObserver(name, description, "none", true, &DoubleObserverConstructorCallback);
                dsobs->observe(val, labelkv);
            } else if (instrument == "dudobs"){
                auto dudobs= m->NewDoubleUpDownSumObserver(name, description, "none", true, &DoubleObserverConstructorCallback);
                dudobs->observe(val, labelkv);
            } else if (instrument == "dvobs"){
                auto dvobs= m->NewDoubleValueObserver(name, description, "none", true, &DoubleObserverConstructorCallback);
                dvobs->observe(val, labelkv);
            } else {
                std::cerr <<"Bad entry" <<std::endl;
            }
            //usleep(.1*1000000);
        }
        goldenData.close();
        
    }
};


int main(int argc, const char * argv[]) {
    MetricGenerator::generateData();
}
