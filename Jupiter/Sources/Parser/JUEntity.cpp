
#include "JUEntity.h"
#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"

#include <QRegExp>
#include <QStringBuilder>

#define __JUMODULE__ "Entity"

JUEntity::JUEntity(const QString& name)
{
    JUMLog("ctor {%p}.", this);

    m_name = name;
    m_type = JUEntity::EntityTypeNone;

    m_isValid = true;
    m_wasValidated = true;
    if (m_name == "inverter") {
        m_lookupTable["0"] = "1";
        m_lookupTable["1"] = "0";

        addPort("x", "in", "BIT");
        addPort("y", "out", "BIT");
    } else if (m_name == "2band2_or2") {
        m_type = JUEntity::EntityTypeUBS;
        m_lookupTable["0000"] = "1";
        m_lookupTable["0001"] = "1";
        m_lookupTable["0010"] = "1";
        m_lookupTable["0011"] = "1";
        m_lookupTable["0100"] = "1";
        m_lookupTable["0101"] = "0";
        m_lookupTable["0110"] = "0";
        m_lookupTable["0111"] = "0";
        m_lookupTable["1000"] = "1";
        m_lookupTable["1001"] = "0";
        m_lookupTable["1010"] = "0";
        m_lookupTable["1011"] = "0";
        m_lookupTable["1100"] = "1";
        m_lookupTable["1101"] = "0";
        m_lookupTable["1110"] = "0";
        m_lookupTable["1111"] = "0";

        addPort("x1", "in", "BIT");
        addPort("x2", "in", "BIT");
        addPort("x3", "in", "BIT");
        addPort("x4", "in", "BIT");
        addPort("y", "out", "BIT");
    } else {
        m_isValid = false;
        m_wasValidated = false;
    }
}

JUEntity::~JUEntity()
{
    JUMLog("dtor {%p}.", this);

    m_portsIn.clear();
    m_portsOut.clear();
    m_components.clear();
}

// ========================================

void JUEntity::setType(JUEntity::EntityType type)
{
    m_type = type;
}

void JUEntity::addPort(const QString& name, const QString& mode, const QString& type)
{
    if (type.contains("vector")) {
        QRegExp rx("(\\d)+");
        int pos = 0;
        pos = rx.indexIn(type, pos);
        int first = rx.cap(1).toInt();
        pos += rx.matchedLength();
        pos = rx.indexIn(type, pos);
        int second = rx.cap(1).toInt();

        int count = abs(first - second) + 1;

        for (int i = 0; i < count; ++i) {
            addPort(JUEntity::createPort("ju_" % name % QString::number(i + 1), mode, "BIT"));
        }
    } else {
        addPort(JUEntity::createPort(name, mode, type));
    }
}

JUEntity::Port JUEntity::createPort(const QString& name, const QString& mode, const QString& type)
{
    Port p;
    p.name = name;
    p.mode = mode;
    p.type = type;
    return p;
}

void JUEntity::addPort(Port p)
{
    if (p.mode == "in") {
        m_portsIn.append(p);
    } else if (p.mode == "out") {
        m_portsOut.append(p);
    }
}

void JUEntity::addMappedSignals(const QString& componentName, QStringList signal, const QString& label)
{
    MappedSignals m;
    m.componentName = componentName;
    //m.signal = signal;
    m.label = label;

    JUEntity *component = componentByName(componentName);
    JUAssert(component != NULL, "can not find component.");

    int i = 0;
    for (; i < component->portsIn().count(); ++i) {
        m.signalIn.append(signal[i]);
    }
    for (int j = 0; j < component->portsOut().count(); ++j) {
        m.signalOut.append(signal[i + j]);
    }

    m_signalsMap.append(m);
    //JUMLog("added signals map => %s: %s (in: %s; out: %s).", (label.isEmpty() ? "<no label>" : Q(label)), Q(componentName), Q(stringListDescription(m.signalIn)), Q(stringListDescription(m.signalOut)));
}

bool JUEntity::hasOutputPort(QString output)
{
    for (int i = 0; i < m_portsOut.count(); ++i) {
        if (m_portsOut[i].name.toLower() == output) {
            return true;
        }
    }
    return false;
}

bool JUEntity::setLUT(QMap<QString, QString> lut)
{
    for (int i = 0; i < lut.count(); ++i) {
        QString input = lut.keys()[i];
        QString output = lut[input];

        if (input.length() != m_portsIn.count() || output.length() != m_portsOut.count()) {
            return false;
        }

        for (int j = 0; j < qMax(input.length(), output.length()); ++j) {
            if (j < input.length()) {
                if (input[j] != '0' && input[j] != '1') {
                    return false;
                }
            }
            if (j < output.length()) {
                if (output[j] != '0' && output[j] != '1') {
                    return false;
                }
            }
        }
    }

    if (lut.count() != pow(2, m_portsIn.count())) {
        return false;
    }

    m_lookupTable = lut;
    return true;
}

// ========================================

bool JUEntity::validate()
{
    if (m_wasValidated) {
        return m_isValid;
    }

    for (int i = 0; i < m_components.count(); ++i) {
        if (!m_components[i]->validate()) {
            return false;
        }
    }

    for (int i = 0; i < m_portsOut.count(); ++i) {
        for (int j = 0; j < m_portsIn.count(); ++j) {
            if (m_portsIn[j].name == m_portsOut[i].name) {
                JUMLog("%s: redefinition of the port %s.", Q(m_name), Q(m_portsIn[i].name));
                return false;
            }
        }
    }

    m_isValid = generateLookupTable();
    if (!m_isValid) {
        JUMLog("%s: validation failed.", Q(m_name));
    }

    if (m_isValid) {
        generateCommonLookupTable();
    }

    m_wasValidated = true;
    return m_isValid;
}

bool JUEntity::generateLookupTable()
{
    JUMLog("%s: generating look-up table.", Q(m_name));
    if (m_type == JUEntity::EntityTypeNone) {
        return false;
    }

    if (m_type == JUEntity::EntityTypeLUT) {
        return generateLUT();
    }

    if (m_type == JUEntity::EntityTypeUBS) {
        return generateUBS();
    }

    return false;
}

bool JUEntity::generateUBS()
{
    int inputs = m_portsIn.count();

    int maxValue = (int)pow(2, inputs);
    for (int i = 0; i < maxValue; ++i) {
        QString input = QString::number(i, 2);
        while (input.length() < inputs) {
            input = "0" % input;
        }
        if (!generateUBSOutputForInput(input)) {
            return false;
        }
    }
    return true;
}

bool JUEntity::generateUBSOutputForInput(QString input)
{
    QMap<QString, QString> memory;

    for (int i = 0; i < m_portsIn.count(); ++i) {
        memory[m_portsIn[i].name] = input[i];
    }

    QList<QString> unknownInputs;
    for (int i = 0; i < m_portsOut.count(); ++i) {
        unknownInputs.append(m_portsOut[i].name);
    }

    //JUMLog("input is %s.", Q(input));

    int triesLeft = unknownInputs.count();
    while (unknownInputs.count() > 0) {
        //JUMLog("unknown inputs is %s.", Q(stringListDescription(unknownInputs)));
        //JUMLog("memory is %s", Q(mapStringStringListDescription(memory)));
        QString out = unknownInputs[0];

        bool found = false;
        MappedSignals resultMapping;
        for (int j = 0; j < m_signalsMap.count(); ++j) {
            if (m_signalsMap[j].signalOut.contains(out)) {
                resultMapping = m_signalsMap[j];
                found = true;
                break;
            }
        }

        if (!found) {
            JUMLog("%s: can not find mapping for result signal %s.", Q(m_name), Q(out));
            return false;
        }

        bool added = false;
        for (int j = 0; j < resultMapping.signalIn.count(); ++j) {
            QString signalName = resultMapping.signalIn[j];
            if (signalName != "0" && signalName != "1" && !memory.contains(signalName)) {
                if (!unknownInputs.contains(signalName)) {
                    unknownInputs.append(signalName);
                    added = true;
                }
            }
        }

        if (!added) {
            QString componentInput;
            bool skip = false;
            for (int j = 0; j < resultMapping.signalIn.count(); ++j) {
                QString signalName = resultMapping.signalIn[j];
                if (signalName == "0" || signalName == "1") {
                    componentInput = componentInput % signalName;
                } else {
                    if (!memory.contains(signalName)) {
                        skip = true;
                        triesLeft--;
                        break;
                    } else {
                        componentInput = componentInput % memory[signalName];
                    }
                }
            }

            if (!skip) {
                JUEntity *component = componentByName(resultMapping.componentName);
                JUAssert(component != NULL, "can not find component.");
                QString componentOutput;
                bool success = component->getOutput(componentInput, &componentOutput);
                if (!success) {
                    JUMLog("%s: component %s can not produce result for signal %s.", Q(m_name), Q(component->name()), Q(out));
                    return false;
                }
                memory[out] = componentOutput[resultMapping.signalOut.indexOf(out)];
                unknownInputs.removeFirst();
            } else {
                unknownInputs.removeFirst();
                unknownInputs.append(out);
            }
        } else {
            unknownInputs.removeFirst();
            unknownInputs.append(out);

            triesLeft = unknownInputs.count();
        }

        if (triesLeft == 0) {
            break;
        }
    }

    if (unknownInputs.count() > 0) {
        JUMLog("%s: can not resolve signals: %s.", Q(m_name), Q(stringListDescription(unknownInputs)));
        return false;
    }

    QString output;
    for (int i = 0; i < m_portsOut.count(); ++i) {
        output = output % memory[m_portsOut[i].name];
    }
    m_lookupTable[input] = output;

    return true;
}

bool JUEntity::generateLUT()
{
    return (m_lookupTable.count() > 0);
}

bool JUEntity::generateCommonLookupTable()
{
    JUMLog("%s: generating common look-up table.", Q(m_name));
    if (m_portsOut.count() != 1) {
        return false;
    }

    for (int i = 0; i < m_lookupTable.count(); ++i) {
        QString key = m_lookupTable.keys()[i];
        if (m_lookupTable[key] == "0") {
            continue;
        }

        QString elem;
        for (int j = 0; j < key.length(); ++j) {
            QChar c = key[j];
            elem = elem % (c == '1' ? "1" : "0");
        }

        m_commonLookupTable.append(elem);
    }

    QMap<int, QList<QString>> groups;
    for (int i = 0; i < m_commonLookupTable.count(); ++i) {
        QString e = m_commonLookupTable[i];
        groups[e.count("1")].append(e);
    }

    QList<QString> impls;
    bool process = true;
    while (process) {
        bool joined = false;
        QMap<int, QList<QString>> new_groups;
        QMap<int, QList<QString>> tmp_groups = groups;
        for (int i = 0; i < groups.keys().count() - 1; ++i) {
            QList<QString> fg = groups[groups.keys()[i]];
            QList<QString> sg = groups[groups.keys()[i + 1]];

            for (int j = 0; j < fg.count(); ++j) {
                for (int k = 0; k < sg.count(); ++k) {
                    QString res;
                    if (join(fg[j],sg[k], &res)) {
                        if (tmp_groups[tmp_groups.keys()[i]].contains(fg[j])) {
                            tmp_groups[tmp_groups.keys()[i]].removeOne(fg[j]);
                        }
                        if (tmp_groups[tmp_groups.keys()[i + 1]].contains(sg[k])) {
                            tmp_groups[tmp_groups.keys()[i + 1]].removeOne(sg[k]);
                        }
                        int cnt = res.count("1");
                        if (!new_groups.keys().contains(cnt) || !new_groups[cnt].contains(res)) {
                            new_groups[cnt].append(res);
                        }
                        joined = true;
                    }
                }
            }
        }

        for (int i = 0; i <= tmp_groups.keys().count() - 1; ++i) {
            QList<QString> fg = tmp_groups[tmp_groups.keys()[i]];
            for (int j = 0; j < tmp_groups[tmp_groups.keys()[i]].count(); ++j) {
                if (!impls.contains(tmp_groups[tmp_groups.keys()[i]][j])) {
                    impls.append(tmp_groups[tmp_groups.keys()[i]][j]);
                }
            }

            if (i < tmp_groups.keys().count() - 1) {
                QList<QString> sg = tmp_groups[tmp_groups.keys()[i + 1]];
                for (int j = 0; j < tmp_groups[tmp_groups.keys()[i + 1]].count(); ++j) {
                    if (!impls.contains(tmp_groups[tmp_groups.keys()[i + 1]][j])) {
                        impls.append(tmp_groups[tmp_groups.keys()[i + 1]][j]);
                    }
                }
            }
        }

        groups = new_groups;

        process = joined;
    }

    QMap<QString, QSet<int>> map;
    for (int i = 0; i < impls.count(); ++i) {
        QString impl = impls[i];
        map[impl] = QSet<int>();
        for (int j = 0; j < m_commonLookupTable.count(); ++j) {
            if (represents(impl, m_commonLookupTable[j])) {
                map[impl].insert(j);;
            }
        }
    }

    QList<QString> resultImpls;

    while (impls.count() > 0) {
        int max = 0;
        int idx = 0;
        for (int i = 0; i < impls.count(); ++i) {
            if (map[impls[i]].count() > max) {
                idx = i;
                max = map[impls[i]].count();
            }
        }

        resultImpls.append(impls[idx]);
        QString key = impls[idx];
        for (int i = 0; i < map.count(); ++i) {
            map[map.keys()[i]] -= map[key];
        }
        map.remove(key);
        impls.removeAt(idx);
    }

    m_commonLookupTable = resultImpls;
    return true;
}

// ========================================

bool JUEntity::getOutput(QString input, QString *output)
{
    if (!m_isValid || !output) {
        return false;
    }

    if (input.length() != m_portsIn.count()) {
        return false;
    }

    if (!m_lookupTable.contains(input)) {
        return false;
    }

    *output = m_lookupTable[input];
    return true;
}

// ========================================

QString JUEntity::description()
{
    QString desc = QString("entity \"%1\":\n").arg(m_name);

    QString portInDesc = QString(" | ports in => ");
    for (int i = 0; i < m_portsIn.count(); ++i) {
        Port p = m_portsIn[i];
        portInDesc = portInDesc % p.name % ": " % p.mode % " " % p.type % "; ";
    }
    desc = desc % portInDesc % "\n";
    QString portOutDesc = QString(" | ports out => ");
    for (int i = 0; i < m_portsOut.count(); ++i) {
        Port p = m_portsOut[i];
        portOutDesc = portOutDesc % p.name % ": " % p.mode % " " % p.type;
    }
    desc = desc % portOutDesc % "\n";
    QString usedComponentsDesc = QString(" | used components => ");
    for (int i = 0; i < m_components.count(); ++i) {
        usedComponentsDesc = usedComponentsDesc % m_components[i]->name() % (i == m_components.count() - 1 ? "" : ", ");
    }
    desc = desc % usedComponentsDesc % "\n";
    QString declaredSignalsDesc = QString(" | declared signals => ");
    for (int i = 0; i < m_declaredSignals.count(); ++i) {
        Port s = m_declaredSignals[i];
        declaredSignalsDesc = declaredSignalsDesc % s.name % ": " % s.type % "; ";
    }
    desc = desc % declaredSignalsDesc % "\n";
    if (m_type == EntityTypeUBS) {
        QString signalsMapDesc = QString(" | signals map =>\n");
        for (int i = 0; i < m_signalsMap.count(); ++i) {
            MappedSignals ms = m_signalsMap[i];
            signalsMapDesc = signalsMapDesc % " |                " % (ms.label.isEmpty() ? "<no label>" : ms.label) % ": " % ms.componentName % " (in: " % stringListDescription(ms.signalIn) % "; out: " % stringListDescription(ms.signalOut) % ")\n";
        }
        desc = desc % signalsMapDesc;
    }
    if (m_isValid) {
        QString lutDesc = QString(" | look-up table =>\n");
        for (int i = 0; i < m_lookupTable.count(); ++i) {
            QString key = m_lookupTable.keys()[i];
            lutDesc = lutDesc % " |                   " % key % " => " % m_lookupTable[key] % "\n";
        }
        desc = desc % lutDesc;
        QString clutDesc = QString(" | common look-up table =>\n");
        for (int i = 0; i < m_commonLookupTable.count(); ++i) {
            clutDesc = clutDesc % " |                         " % m_commonLookupTable[i] % "\n";
        }
        desc = desc % clutDesc;
    }

    return desc;
}

JUEntity* JUEntity::componentByName(const QString& name)
{
    for (int i = 0; i < m_components.count(); ++i) {
        if (m_components[i]->name() == name) {
            return m_components[i];
        }
    }
    return NULL;
}

QString JUEntity::stringListDescription(QStringList list)
{
    QString desc;
    if (list.count() > 0) {
        desc = list[0];
    }
    for (int i = 1; i < list.count(); ++i) {
        desc = desc % ", " % list[i];
    }
    return desc;
}

QString JUEntity::intListDescription(QList<int> list)
{
    QString desc;
    if (list.count() > 0) {
        desc = QString::number(list[0]);
    }
    for (int i = 1; i < list.count(); ++i) {
        desc = desc % ", " % QString::number(list[i]);
    }
    return desc;
}

QString JUEntity::mapStringStringListDescription(QMap<QString, QString> map)
{
    QString desc;
    if (map.count() > 0) {
        QString key = map.keys()[0];
        desc = key % " -> " % map[key];
    }
    for (int i = 1; i < map.count(); ++i) {
        QString key = map.keys()[i];
        desc = desc % ", " % key % " -> " % map[key];
    }
    return desc;
}

QString JUEntity::mapIntStringListDescription(QMap<int, QList<QString>> map)
{
    QString desc;
    if (map.count() > 0) {
        int key = map.keys()[0];
        desc = QString("%1").arg(key) % " -> " % stringListDescription(map[key]);
    }
    for (int i = 1; i < map.count(); ++i) {
        int key = map.keys()[i];
        desc = desc % "\n" % QString("%1").arg(key) % " -> " % stringListDescription(map[key]);
    }
    return desc;
}

QString JUEntity::mapStringIntSetDescription(QMap<QString, QSet<int>> map) {
    QString desc;
    if (map.count() > 0) {
        QString key = map.keys()[0];
        desc = QString("%1").arg(key) % " -> (" % QString::number(map[key].count()) % ") " % intListDescription(map[key].toList());
    }
    for (int i = 1; i < map.count(); ++i) {
        QString key = map.keys()[i];
        desc = desc % "\n" % QString("%1").arg(key) % " -> (" % QString::number(map[key].count()) % ") " % intListDescription(map[key].toList());
    }
    return desc;
}

bool JUEntity::represents(QString x, QString y)
{
    if (x.length() != y.length()) {
        return false;
    }

    for (int i = 0; i < x.length(); ++i) {
        if (x[i] != y[i] && x[i] != '-' && y[i] != '-') {
            return false;
        }
    }

    return true;
}

bool JUEntity::join(QString x, QString y, QString *result)
{
    if (x.length() != y.length()) {
        return false;
    }

    QString res = x;
    for (int i = 0; i < x.length(); ++i) {
        if (x[i] != y[i] && (x[i] == '-' || y[i] == '-')) {
            return false;
        }
        if (x[i] == y[i]) {
            continue;
        }
        if (cutChar(x, i) == cutChar(y, i)) {
            res = res.replace(i, 1, '-');
        }
    }

    if (res != x) {
        if (result) {
            *result = res;
        }
        return true;
    }
    return false;
}

QString JUEntity::cutChar(QString s, int pos)
{
    if (s.length() < pos) {
        return s;
    }

    return s.remove(pos, 1);
}

// PORT =======================================================================

JUEntity::Port::Port()
{

}