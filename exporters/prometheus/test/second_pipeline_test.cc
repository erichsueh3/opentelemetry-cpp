#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <utility>
#include <unistd.h>

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
        static std::vector<std::pair<std::string, std::vector<std::string>>> ReadLines() {
            std::ifstream goldenData;
            goldenData.open("exporters/prometheus/test/PrometheusDataSecond.csv");
        

            std::vector<std::pair<std::string, std::vector<std::string>>> lines;
            // std::vector<metric_sdk::Record> records;


            while(goldenData.good() && lines.size() != 2048) {
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

                auto p = std::make_pair(agg, fields);
                lines.push_back(p);
                // metric_sdk::Record r = LineToRecord(agg, fields);
                // records.push_back(r);
            }
            goldenData.close();

            return lines;
        }


        static std::vector<metric_sdk::Record> CreateRecords(std::vector<std::pair<std::string, std::vector<std::string>>> lines) {
            std::vector<metric_sdk::Record> records;

            for (auto l : lines) {
                std::string agg = l.first;
                std::vector<std::string> v = l.second;

                std::string name;
                std::string description;
                std::string labels;
                std::shared_ptr<metric_sdk::Aggregator<double>> aggregator;

                if(agg == "counter" || agg == "gauge") {
                    if(agg == "counter") {
                        aggregator = CreateAgg(metric_sdk::AggregatorKind::Counter);
                    }
                    else {
                        aggregator = CreateAgg(metric_sdk::AggregatorKind::Gauge);
                    }

                    int val;
                    CounterGaugeFieldTranslation(v, val, name, description, labels);
                    // std::cout << "name:"<< name << "|description:" << description << "|labels:" << labels << std::endl;

                    aggregator->update(val);
                    aggregator->checkpoint();
                }
                
                else {
                    std::vector<int> vals;
                    std::vector<double> etc; // boundaries or quantiles
                    HistogramSketchExactFieldTranslation(v, vals, etc, name, description, labels);

                    if(agg == "histogram") {
                        aggregator = CreateAgg(metric_sdk::AggregatorKind::Histogram);
                    }
                    else if(agg == "sketch") {
                        aggregator = CreateAgg(metric_sdk::AggregatorKind::Sketch);
                    }
                    else {
                        aggregator = CreateAgg(metric_sdk::AggregatorKind::Exact);
                    }


                }

                metric_sdk::Record r{name, description, labels, aggregator};
                records.push_back(r);
            }
            return records;
        }

    private:
        static std::shared_ptr<metric_sdk::Aggregator<double>> CreateAgg(metric_sdk::AggregatorKind kind, bool exactMode = true) {
            std::shared_ptr<metric_sdk::Aggregator<double>> aggregator;
            switch (kind)
            {
                case metric_sdk::AggregatorKind::Counter:
                {
                aggregator = std::shared_ptr<metric_sdk::Aggregator<double>>(
                    new metric_sdk::CounterAggregator<double>(opentelemetry::metrics::InstrumentKind::Counter));
                break;
                }
                case metric_sdk::AggregatorKind::MinMaxSumCount:
                {
                aggregator =
                    std::shared_ptr<metric_sdk::Aggregator<double>>(new metric_sdk::MinMaxSumCountAggregator<double>(
                        opentelemetry::metrics::InstrumentKind::Counter));
                break;
                }
                case metric_sdk::AggregatorKind::Gauge:
                {
                aggregator = std::shared_ptr<metric_sdk::Aggregator<double>>(
                    new metric_sdk::GaugeAggregator<double>(opentelemetry::metrics::InstrumentKind::Counter));
                break;
                }
                case metric_sdk::AggregatorKind::Sketch:
                {
                aggregator = std::shared_ptr<metric_sdk::Aggregator<double>>(new metric_sdk::SketchAggregator<double>(
                    opentelemetry::metrics::InstrumentKind::Counter, 0.000005));
                break;
                }
                case metric_sdk::AggregatorKind::Histogram:
                {
                std::vector<double> boundaries{10, 20};
                aggregator =
                    std::shared_ptr<metric_sdk::Aggregator<double>>(new metric_sdk::HistogramAggregator<double>(
                        opentelemetry::metrics::InstrumentKind::Counter, boundaries));
                break;
                }
                case metric_sdk::AggregatorKind::Exact:
                {
                aggregator = std::shared_ptr<metric_sdk::Aggregator<double>>(new metric_sdk::ExactAggregator<double>(
                    opentelemetry::metrics::InstrumentKind::Counter, exactMode));
                break;
                }
                default:
                aggregator = nullptr;
            }
            return aggregator;
            }


        static void CounterGaugeFieldTranslation(std::vector<std::string> fields, int &val, std::string &name, std::string &description, std::string &labels) {
            val = stoi(fields[0]);

            std::string ndl = fields[1];
            NameDesLabTranslation(ndl, name, description, labels);

        }

        static void HistogramSketchExactFieldTranslation(std::vector<std::string> fields, std::vector<int> &vals, std::vector<double> &etc, std::string &name, std::string &description, std::string &labels) {
            std::string str_vals = fields[0];
            std::string str_etc = fields[1];

            StringToIntegralListTranslation<int>(str_vals, vals);
            StringToIntegralListTranslation<double>(str_etc, etc);

            // std::string str_vals = fields[0];

            // str_vals.erase(str_vals.begin());
            // str_vals.erase(str_vals.end()-1);

            // std::stringstream ss1(str_vals);
            // std::string v;
            // while(std::getline(ss1, v, ',')) {
            //     vals.push_back(stoi(v));
            // }


            // std::string str_etc = fields[1];

            // str_etc.erase(str_etc.begin());
            // str_etc.erase(str_etc.end()-1);

            // std::stringstream ss2(str_etc);
            // while(std::getline(ss2, v, ',')) {
            //     etc.push_back(stoi(v));
            // }

            for(auto i: vals) {
                std::cout << ">" << i << "<";
            }
            std::cout << std::endl;
            for(auto i: etc) {
                std::cout << ">" << i << "<";
            }
            std::cout << std::endl;

            std::string ndl = fields[2];
            NameDesLabTranslation(ndl, name, description, labels);
        }

        static void NameDesLabTranslation(std::string &ndl, std::string &name, std::string &description, std::string &labels) {
            name = ndl.substr(0, ndl.find(','));
            description = ndl.substr(name.size() + 2, ndl.find(',', name.size()+1)-ndl.find(',')-2);
            labels = ndl.substr(name.size() + description.size()+4);
        }

        template <typename T>
        static void StringToIntegralListTranslation(std::string &str_v, std::vector<T> &v) {
            str_v.erase(str_v.begin());
            str_v.erase(str_v.end()-1);

            std::stringstream ss(str_v);
            std::string val;
            while(std::getline(ss, val, ',')) {
                v.push_back(stod(val));
            }
        }

};





int main(int argc, const char * argv[]) {
    std::string addr = "localhost:8080";
    prometheus_exporter::PrometheusExporter exporter{addr};

    std::vector<std::pair<std::string, std::vector<std::string>>> lines = RecordGenerator::ReadLines();
    std::vector<metric_sdk::Record> records = RecordGenerator::CreateRecords(lines);

    exporter.Export(records);
    while(true) {
        std::cout << "waiting for scrape" << std::endl;
        usleep(.1*10000000);
    }
}
