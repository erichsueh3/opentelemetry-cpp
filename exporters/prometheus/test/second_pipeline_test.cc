#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>


#include "opentelemetry/exporters/prometheus/prometheus_exporter.h"
#include "opentelemetry/metrics/instrument.h"
#include "opentelemetry/sdk/metrics/aggregator/aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/counter_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/exact_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/gauge_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/histogram_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/min_max_sum_count_aggregator.h"
#include "opentelemetry/sdk/metrics/aggregator/sketch_aggregator.h"
#include "opentelemetry/sdk/metrics/record.h"

namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace prometheus_exporter = opentelemetry::exporter::prometheus;


class RecordGenerator {
    public:
        static void CreateRecords() {
            std::ifstream goldenData;
            goldenData.open("exporters/prometheus/test/PrometheusDataSecond.csv");
        
            std::vector<metric_sdk::Record> records;


            while(goldenData.good() && records.size() != 2048) {
                std::vector<std::string> fields;
                std::string field;
                std::string agg;

                std::getline(goldenData, agg, '|');

                if(agg == "sketch" || agg == "exact" || agg == "histogram") {
                    std::getline(goldenData, field, '|');
                    fields.push_back(field);
                }

                std::getline(goldenData, field, '|');
                fields.push_back(field); 

                std::getline(goldenData, field, '\n');
                fields.push_back(field);    

                // metric_sdk::Record r = LineToRecord(agg, fields);
                // records.push_back(r);
            }

        }

    private:
        static metric_sdk::Record LineToRecord(std::string agg, std::vector<std::string> fields) {
            
        //     if(agg == "counter") {
        //         std::shared_ptr<metric_sdk::Aggregator<double>> aggregator = std::shared_ptr<metric_sdk::Aggregator<double>>(
        //             new metric_sdk::CounterAggregator<double>(opentelemetry::metrics::InstrumentKind::Counter));
                
        //         int val;
        //         std::string name, des, labels;
        //         CounterGaugeFieldTranslation(fields, val, name, des, labels);
        //         metric_sdk::Record record{name, des, labels, aggregator};

        //         std::cout << name << " " << des << " " << labels << std::endl;
        //         return record;
        //     }
        }
        // static void CounterGaugeFieldTranslation(std::vector<std::string> fields, int &val, std::string &name, std::string &des, std::string &labels) {
        //     val = stoi(fields[0]);

        //     std::string ndl = fields[1];

        //     name = ndl.substr(0, ndl.find(','));
        //     des = ndl.substr(name.size() + 1, ndl.find(','));
        //     labels = ndl.substr(name.size() + des.size() + 2);
            
        // }

};

int main(int argc, const char * argv[]) {
    RecordGenerator::CreateRecords();
}
