#pragma once

#include "opentelemetry/metrics/instrument.h"

OPENTELEMETRY_BEGIN_NAMESPACE

namespace sdk
{
namespace metrics
{
class Record
{
public:
  explicit Record(std::string name, std::string description, InstrumentKind instrumentKind,
                  std::map<std::string, std::string> labels,
                  std::variant<std::vector<int>, std::vector<double>> value)
  {
    name_{name};
    description_{description};
    instrumentKind_{instrumentKind};
    labels_{labels};
    value_{value};
  }

  std::string GetName() {return name_;}
  std::string GetDescription() {return description_;}
  InstrumentKind GetInstrumentKind() {return instrumentKind_};
  std::string GetLabels() {return labels_};
  std::variant<std::vector<int>, std::vector<double>> GetValue() {return value_};

private:
  std::string name_;
  std::string description_;
  InstrumentKind instrumentKind_;
  std::string labels_;
  std::variant<std::vector<int>, std::vector<double>> value_;
};
}
}