#include "QtFormsGenerator.h"

#include <algorithm>
#include <climits>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include "pbform/ui_options.pb.h"

namespace pbuf = google::protobuf;
using FD = pbuf::FieldDescriptor;

namespace pbform {
namespace {

// ---------- kleine String-Helfer ----------

std::string Escape(const std::string &s) {
    std::string out;
    for (char c : s) {
        switch (c) {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
        }
    }
    return out;
}

std::string QStr(const std::string &s) {
    return "QStringLiteral(\"" + Escape(s) + "\")";
}
std::string QStr(std::string_view s) {
    return QStr(std::string(s));
}

std::string FmtDouble(double v) {
    std::ostringstream s;
    s.precision(17);
    s << v;
    std::string r = s.str();
    if (r.find_first_of(".e") == std::string::npos)
        r += ".0";
    return r;
}

int ClampInt(double v) {
    return static_cast<int>(std::max<double>(INT_MIN, std::min<double>(INT_MAX, v)));
}

std::string StripProto(const std::string &n) {
    const std::string ext = ".proto";
    if (n.size() >= ext.size() && n.compare(n.size() - ext.size(), ext.size(), ext) == 0)
        return n.substr(0, n.size() - ext.size());
    return n;
}

std::vector<std::string> Split(const std::string &s, char sep) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) {
        if (c == sep) {
            if (!cur.empty())
                parts.push_back(cur);
            cur.clear();
        } else
            cur += c;
    }
    if (!cur.empty())
        parts.push_back(cur);
    return parts;
}

std::string NsOpen(const std::string &package) {
    std::string r;
    for (const auto &p : Split(package, '.'))
        r += "namespace " + p + " {\n";
    return r;
}

std::string NsClose(const std::string &package) {
    std::string r;
    auto parts = Split(package, '.');
    for (auto it = parts.rbegin(); it != parts.rend(); ++it)
        r += "}  // namespace " + *it + "\n";
    return r;
}

std::string NsPrefix(const std::string &package) {
    std::string r;
    for (const auto &p : Split(package, '.'))
        r += "::" + p;
    return r;
}
std::string NsPrefix(std::string_view package) {
    return NsPrefix(std::string(package));
}

// "pkg.Outer.Inner" -> "::pkg::Outer_Inner" (so benennt protoc die C++-Typen)
std::string Qualified(const std::string &package, const std::string &fullName) {
    std::string rest = package.empty() ? fullName : fullName.substr(package.size() + 1);
    std::replace(rest.begin(), rest.end(), '.', '_');
    return NsPrefix(package) + "::" + rest;
}

std::string FormClass(const pbuf::Descriptor *d) {
    return NsPrefix(d->file()->package()) + "::" + std::string(d->name()) + "Form";
}

bool StartsWith(const std::string &s, const std::string &p) {
    return s.rfind(p, 0) == 0;
}

bool Write(pbuf::compiler::GeneratorContext *ctx, const std::string &name, const std::string &content) {
    std::unique_ptr<pbuf::io::ZeroCopyOutputStream> out(ctx->Open(name));
    if (!out)
        return false;
    pbuf::io::CodedOutputStream coded(out.get());
    coded.WriteRaw(content.data(), static_cast<int>(content.size()));
    return !coded.HadError();
}

// ---------- Code pro Feld ----------

struct FieldCode {
    std::string member; // Deklaration im Header
    std::string ctor;   // Widget-Erzeugung im Konstruktor
    std::string load;   // Message -> UI
    std::string store;  // UI -> Message
};

FieldCode GenField(const FD *f, const pbuf::Descriptor *owner, std::set<std::string> &includes) {
    FieldCode o;
    const auto &ui = f->options().GetExtension(pbform::field_ui);
    const std::string label = std::string(ui.label().empty() ? f->name() : ui.label());
    const std::string ln = std::string(f->lowercase_name());
    const std::string w = "w_" + ln;

    auto unsupported = [&](const std::string &why) {
        o.ctor = "        {\n"
                 "            auto* note = new QLabel(" +
                 QStr("(" + why + " - not supported yet)") +
                 ");\n"
                 "            note->setEnabled(false);\n"
                 "            form->addRow(" +
                 QStr(label) +
                 ", note);\n"
                 "        }\n";
        return o;
    };

    auto tail = [&]() {
        std::string s;
        if (!ui.tooltip().empty())
            s += "        " + w + "->setToolTip(" + QStr(ui.tooltip()) + ");\n";
        if (ui.read_only())
            s += "        " + w + "->setEnabled(false);\n";
        s += "        form->addRow(" + QStr(label) + ", " + w + ");\n";
        return s;
    };

    if (f->is_repeated())
        return unsupported("repeated/map");
    if (f->real_containing_oneof())
        return unsupported("oneof");
    if (f->type() == FD::TYPE_BYTES)
        return unsupported("bytes");

    switch (f->cpp_type()) {
    case FD::CPPTYPE_INT32:
    case FD::CPPTYPE_UINT32: {
        const bool u = f->cpp_type() == FD::CPPTYPE_UINT32;
        o.member = "    QSpinBox* " + w + " = nullptr;\n";
        o.ctor = "        " + w + " = new QSpinBox;\n";
        o.ctor += "        " + w + "->setRange(" +
                  std::to_string(ui.has_min() ? ClampInt(ui.min()) : (u ? 0 : INT_MIN)) + ", " +
                  std::to_string(ui.has_max() ? ClampInt(ui.max()) : INT_MAX) + ");\n";
        if (!ui.unit().empty())
            o.ctor += "        " + w + "->setSuffix(" + QStr(" " + ui.unit()) + ");\n";
        o.ctor += tail();
        o.load = "    " + w + "->setValue(static_cast<int>(msg." + ln + "()));\n";
        o.store = "    msg.set_" + ln + "(static_cast<" + (u ? "uint32_t" : "int32_t") + ">(" + w +
                  "->value()));\n";
        return o;
    }
    case FD::CPPTYPE_INT64:
    case FD::CPPTYPE_UINT64:
    case FD::CPPTYPE_FLOAT:
    case FD::CPPTYPE_DOUBLE: {
        const auto t = f->cpp_type();
        const bool isInt = (t == FD::CPPTYPE_INT64 || t == FD::CPPTYPE_UINT64);
        const std::string setT = t == FD::CPPTYPE_INT64    ? "int64_t"
                                 : t == FD::CPPTYPE_UINT64 ? "uint64_t"
                                 : t == FD::CPPTYPE_FLOAT  ? "float"
                                                           : "double";
        o.member = "    QDoubleSpinBox* " + w + " = nullptr;\n";
        o.ctor = "        " + w + " = new QDoubleSpinBox;\n";
        o.ctor += "        " + w + "->setDecimals(" + (isInt ? "0" : "3") + ");\n";
        o.ctor += "        " + w + "->setRange(" +
                  FmtDouble(ui.has_min() ? ui.min() : (t == FD::CPPTYPE_UINT64 ? 0.0 : -9e15)) + ", " +
                  FmtDouble(ui.has_max() ? ui.max() : 9e15) + ");\n";
        if (!ui.unit().empty())
            o.ctor += "        " + w + "->setSuffix(" + QStr(" " + ui.unit()) + ");\n";
        o.ctor += tail();
        o.load = "    " + w + "->setValue(static_cast<double>(msg." + ln + "()));\n";
        o.store = "    msg.set_" + ln + "(static_cast<" + setT + ">(" + w + "->value()));\n";
        return o;
    }
    case FD::CPPTYPE_BOOL:
        o.member = "    QCheckBox* " + w + " = nullptr;\n";
        o.ctor = "        " + w + " = new QCheckBox;\n" + tail();
        o.load = "    " + w + "->setChecked(msg." + ln + "());\n";
        o.store = "    msg.set_" + ln + "(" + w + "->isChecked());\n";
        return o;
    case FD::CPPTYPE_STRING:
        if (ui.widget() == "textarea") {
            o.member = "    QPlainTextEdit* " + w + " = nullptr;\n";
            o.ctor = "        " + w + " = new QPlainTextEdit;\n        " + w + "->setMaximumHeight(80);\n" +
                     tail();
            o.load = "    " + w + "->setPlainText(QString::fromStdString(msg." + ln + "()));\n";
            o.store = "    msg.set_" + ln + "(" + w + "->toPlainText().toStdString());\n";
        } else {
            o.member = "    QLineEdit* " + w + " = nullptr;\n";
            o.ctor = "        " + w + " = new QLineEdit;\n" + tail();
            o.load = "    " + w + "->setText(QString::fromStdString(msg." + ln + "()));\n";
            o.store = "    msg.set_" + ln + "(" + w + "->text().toStdString());\n";
        }
        return o;
    case FD::CPPTYPE_ENUM: {
        const auto *e = f->enum_type();
        o.member = "    QComboBox* " + w + " = nullptr;\n";
        o.ctor = "        " + w + " = new QComboBox;\n";
        for (int i = 0; i < e->value_count(); ++i)
            o.ctor += "        " + w + "->addItem(" + QStr(e->value(i)->name()) + ", " +
                      std::to_string(e->value(i)->number()) + ");\n";
        o.ctor += tail();
        o.load = "    " + w + "->setCurrentIndex(" + w + "->findData(static_cast<int>(msg." + ln + "())));\n";
        o.store = "    msg.set_" + ln + "(static_cast<" +
                  Qualified(std::string(e->file()->package()), std::string(e->full_name())) + ">(" + w +
                  "->currentData().toInt()));\n";
        return o;
    }
    case FD::CPPTYPE_MESSAGE: {
        const auto *md = f->message_type();
        if (md->containing_type() != nullptr ||
            StartsWith(std::string(md->file()->name()), "google/protobuf/"))
            return unsupported("nested/well-known type");
        if (md == owner)
            return unsupported("recursive type");
        if (md->file() != owner->file())
            includes.insert(StripProto(std::string(md->file()->name())) + ".qtform.h");
        const std::string cls = FormClass(md);
        o.member = "    " + cls + "* " + w + " = nullptr;\n";
        o.ctor = "        {\n"
                 "            auto* sub = new QGroupBox(" +
                 QStr(label) +
                 ");\n"
                 "            auto* l = new QVBoxLayout(sub);\n"
                 "            " +
                 w + " = new " + cls +
                 ";\n"
                 "            l->addWidget(" +
                 w + ");\n";
        if (ui.read_only())
            o.ctor += "            sub->setEnabled(false);\n";
        o.ctor += "            form->addRow(sub);\n        }\n";
        o.load = "    " + w + "->setMessage(msg." + ln + "());\n";
        o.store = "    " + w + "->getMessage(*msg.mutable_" + ln + "());\n";
        return o;
    }
    }
    return unsupported("unknown");
}

// ---------- Code pro Message ----------

void GenMessage(const pbuf::Descriptor *d, std::ostringstream &h, std::ostringstream &c,
                std::set<std::string> &includes) {
    const std::string cls = std::string(d->name()) + "Form";
    const std::string msgT = Qualified(std::string(d->file()->package()), std::string(d->full_name()));
    const auto &mu = d->options().GetExtension(pbform::message_ui);

    std::vector<const FD *> fields;
    for (int i = 0; i < d->field_count(); ++i)
        fields.push_back(d->field(i));
    std::stable_sort(fields.begin(), fields.end(), [](const FD *a, const FD *b) {
        return a->options().GetExtension(pbform::field_ui).order() <
               b->options().GetExtension(pbform::field_ui).order();
    });

    std::vector<std::pair<std::string, std::vector<const FD *>>> groups;
    for (const FD *f : fields) {
        const auto &g = f->options().GetExtension(pbform::field_ui).group();
        auto it = std::find_if(groups.begin(), groups.end(), [&](const auto &p) { return p.first == g; });
        if (it == groups.end())
            groups.push_back({g, {f}});
        else
            it->second.push_back(f);
    }

    std::ostringstream members, ctor, load, store;
    for (const auto &g : groups) {
        ctor << "    {\n"
             << "        auto* box = new QGroupBox(" << QStr(g.first) << ");\n"
             << "        auto* form = new QFormLayout(box);\n"
             << "        root->addWidget(box);\n";
        for (const FD *f : g.second) {
            FieldCode fc = GenField(f, d, includes);
            members << fc.member;
            ctor << fc.ctor;
            load << fc.load;
            store << fc.store;
        }
        ctor << "    }\n";
    }

    h << "class " << cls << " : public QWidget {\n"
      << "public:\n"
      << "    explicit " << cls << "(QWidget* parent = nullptr);\n"
      << "    void setMessage(const " << msgT << "& msg);\n"
      << "    void getMessage(" << msgT << "& msg) const;\n\n"
      << "private:\n"
      << members.str() << "};\n\n";

    c << cls << "::" << cls << "(QWidget* parent) : QWidget(parent) {\n"
      << "    auto* root = new QVBoxLayout(this);\n"
      << "    root->setContentsMargins(0, 0, 0, 0);\n";
    if (!mu.title().empty())
        c << "    setWindowTitle(" << QStr(mu.title()) << ");\n";
    c << ctor.str() << "}\n\n"
      << "void " << cls << "::setMessage(const " << msgT << "& msg) {\n"
      << "    static_cast<void>(msg);\n"
      << load.str() << "}\n\n"
      << "void " << cls << "::getMessage(" << msgT << "& msg) const {\n"
      << "    static_cast<void>(msg);\n"
      << store.str() << "}\n\n";
}

} // namespace

uint64_t QtFormsGenerator::GetSupportedFeatures() const {
    return FEATURE_PROTO3_OPTIONAL;
}

bool QtFormsGenerator::Generate(const pbuf::FileDescriptor *file, const std::string & /*parameter*/,
                                pbuf::compiler::GeneratorContext *ctx, std::string *error) const {
    if (StartsWith(std::string(file->name()), "google/protobuf/") || file->message_type_count() == 0)
        return true;

    const std::string base = StripProto(std::string(file->name()));
    const std::string pkg = std::string(file->package());
    std::set<std::string> includes;
    std::ostringstream fwd, hdefs, cdefs;

    for (int i = 0; i < file->message_type_count(); ++i)
        fwd << "class " << file->message_type(i)->name() << "Form;\n";
    for (int i = 0; i < file->message_type_count(); ++i)
        GenMessage(file->message_type(i), hdefs, cdefs, includes);

    std::ostringstream h;
    h << "// Generated by protoc-gen-qtforms from " << file->name() << ". DO NOT EDIT.\n"
      << "#pragma once\n\n"
      << "#include <QWidget>\n"
      << "#include \"" << base << ".pb.h\"\n";
    for (const auto &inc : includes)
        h << "#include \"" << inc << "\"\n";
    h << "\nclass QCheckBox;\nclass QComboBox;\nclass QDoubleSpinBox;\nclass QLineEdit;\n"
      << "class QPlainTextEdit;\nclass QSpinBox;\n\n"
      << NsOpen(pkg) << "\n"
      << fwd.str() << "\n"
      << hdefs.str() << NsClose(pkg);

    std::ostringstream c;
    c << "// Generated by protoc-gen-qtforms from " << file->name() << ". DO NOT EDIT.\n"
      << "#include \"" << base << ".qtform.h\"\n\n"
      << "#include <QCheckBox>\n#include <QComboBox>\n#include <QDoubleSpinBox>\n#include <QFormLayout>\n"
      << "#include <QGroupBox>\n#include <QLabel>\n#include <QLineEdit>\n#include <QPlainTextEdit>\n"
      << "#include <QSpinBox>\n#include <QVBoxLayout>\n\n"
      << NsOpen(pkg) << "\n"
      << cdefs.str() << NsClose(pkg);

    if (!Write(ctx, base + ".qtform.h", h.str()) || !Write(ctx, base + ".qtform.cpp", c.str())) {
        *error = "qtforms: konnte Ausgabedateien für " + std::string(file->name()) + " nicht schreiben";
        return false;
    }
    return true;
}

} // namespace pbform
