#pragma once
#include <cstdint>
#include <string>

#include <google/protobuf/compiler/code_generator.h>

namespace pbform {

// Erzeugt pro .proto-Datei <name>.qtform.h/.cpp mit je einer QWidget-Klasse
// "<Message>Form" pro Top-Level-Message.
class QtFormsGenerator : public google::protobuf::compiler::CodeGenerator {
public:
    bool Generate(const google::protobuf::FileDescriptor* file,
                  const std::string& parameter,
                  google::protobuf::compiler::GeneratorContext* context,
                  std::string* error) const override;

    uint64_t GetSupportedFeatures() const override;
};

}  // namespace pbform
