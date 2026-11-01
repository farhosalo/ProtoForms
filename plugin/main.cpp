#include <google/protobuf/compiler/plugin.h>

#include "QtFormsGenerator.h"

int main(int argc, char* argv[]) {
    pbform::QtFormsGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
