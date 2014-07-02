
#include "JUReconfUBS.h"
#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"
 
#include <QStringBuilder>

#define LUT_ELEM_WIDTH 50
#define LUT_ELEM_MARGIN 5
#define DISTANCE_BETWEEN_LEVELS_X 100
#define DISTANCE_BETWEEN_LUTS_Y 100
#define DISTANCE_BETWEEN_LUT_PORTS 25
#define DISTANCE_MIN_BETWEEN_INPUTS 30
#define PIXMAP_MARGIN 10
#define INPUT_ELEM_WIDTH 30
#define INPUT_ELEM_HEIGHT 20
#define CROSS_WIDTH 8
#define CROSS_HEIGHT 8
#define DISTANCE_BETWEEN_LINES 10
#define CIRCLE_RADIUS 3

#define __JUMODULE__ "ReconfUBS"

JUReconfUBS::JUReconfUBS(JUEntity *mainEntity, QList<JUSchemeErrorUBS> errors, int elemsCount, int reservedElemsCount)
{
    m_errors = errors;
    m_elemsCount = elemsCount;
    m_reservedElemsCount = reservedElemsCount;
    m_elemPortsCount = 4;
    m_mainEntity = mainEntity;

    m_ubsElemHeight = 25 * (m_elemPortsCount - 1) + 2 * LUT_ELEM_MARGIN;

    QList<JUEntity::MappedSignals> signalsMap = m_mainEntity->singnalsMap();
    int normalElemsCount = elemsCount - errors.count() + reservedElemsCount;
    int i = 0;
    for (; i < normalElemsCount && i < signalsMap.count(); ++i) {
        JUEntity::MappedSignals ms = signalsMap[i];
        QString name = "ubs" % QString::number(i);
        JUProtoUBS u(name, JUSchemeErrorUBS(), ms.signalOut[0]);

        for (int j = 0; j < ms.signalIn.length(); ++j) {
            u.mapPortToSignal(j, ms.signalIn[j]);
        }

        m_mapped.append(u);
    }
    for (; i < signalsMap.count(); ++i) {
        m_notMappedEntitySignals.append(signalsMap[i]);
    }
    if (m_notMappedEntitySignals.count() > 0) {
        for (int i = 0; i < errors.count(); ++i) {
            QString name = "eubs" % QString::number(i);
            JUProtoUBS u(name, errors[i], name);
            m_broken.append(u);
        }
    }
}

JUReconfUBS::~JUReconfUBS()
{
}

// ========================================

bool JUReconfUBS::configure()
{
    if (m_notMappedEntitySignals.count() > 0) {
        int twoCount = 0;
        int oneTwoCount = 0;
        int andOrCount = 0;

        QList<JUProtoUBS> listAndOr;
        QList<JUProtoUBS> listTwo;
        QList<JUProtoUBS> listOneTwo;

        for (int i = 0; i < m_broken.count(); ++i) {
            JUProtoUBS u = m_broken[i];
            if (u.type() == JUProtoUBS::AndOr) {
                andOrCount++;
                listAndOr.append(u);
            } else if (u.type() == JUProtoUBS::Two) {
                twoCount++;
                listTwo.append(u);
            } else if (u.type() == JUProtoUBS::OneTwo) {
                oneTwoCount++;
                listOneTwo.append(u);
            }
        }

        int fullElementsNeededCount = m_notMappedEntitySignals.count();
        
        int allCount = m_broken.count();
        int maxAndOrCount = 0;
        for (int i = 0; i < andOrCount; ++i) {
            if ((maxAndOrCount + 1) * 5 < allCount) {
                maxAndOrCount++;
            } else {
                break;
            }
        }
        int maxOneTwoCount = 0;
        for (int i = 0; i < oneTwoCount; ++i) {
            if ((maxOneTwoCount + 1) * 3 < allCount) {
                maxOneTwoCount++;
            } else {
                break;
            }
        }
        int maxTwoCount = allCount / 4;

        ThreeInt tiAndOr;
        ThreeInt tiOneTwo;

        bool stop = false;
        for (int i = 0; i <= maxAndOrCount; ++i) {
            int tTwoCount = twoCount;
            int tOneTwoCount = oneTwoCount;
            int tAndOrCount = andOrCount;

            ThreeInt ti;
            ti.type = 1;
            tAndOrCount -= i;
            ti.andor = i;
            ti.onetwo = 0;
            ti.self = 0;
            ti.two = 0;
            for (int f = 0; f < i * 4; ++f) {
                if (tTwoCount > 0) {
                    tTwoCount--;
                    ti.two++;
                } else if (tOneTwoCount > 0) {
                    tOneTwoCount--;
                    ti.onetwo++;
                } else {
                    tTwoCount--;
                    ti.self++;
                }
            }
            for (int j = 0; j <= qMin(maxOneTwoCount, tOneTwoCount); ++j) {
                ThreeInt ti1;
                ti1.type = 2;
                ti1.onetwo = j;
                ti1.andor = 0;
                ti1.self = 0;
                ti1.two = 0;
                tOneTwoCount -= j;
                for (int f = 0; f < j * 2; ++f) {
                    if (tTwoCount > 0) {
                        tTwoCount--;
                        ti1.two++;
                    } else if (tAndOrCount > 0) {
                        tAndOrCount--;
                        ti1.andor++;
                    } else {
                        tOneTwoCount--;
                        ti1.self++;
                    }
                }
                for (int k = 0; k <= maxTwoCount; ++k) {
                    int tAllCount = tTwoCount + tAndOrCount + tOneTwoCount;
                    int elemsCount = i + j + tAllCount / 4;
                    if (elemsCount >= fullElementsNeededCount) {
                        tiAndOr = ti;
                        tiOneTwo = ti1;
                        stop = true;
                        break;
                    }
                }
                if (stop) {
                    break;
                }
            }
            if (stop) {
                break;
            }
        }

        if (!stop) {
            m_errorMsg = "Not enough elements.";
            JUMLog("not enough elements.");
            return false;
        }

        if (stop) {
            for (int i = 0; i < tiAndOr.andor; ++i) {
                if (m_notMappedEntitySignals.count() == 0) break;
                JUProtoUBS andOr = listAndOr.first();
                listAndOr.removeFirst();

                QList<JUProtoUBS> helpers;
                int cnt = 0;
                while (cnt < 4 && tiAndOr.two > 0) {
                    helpers.append(listTwo.first());
                    listTwo.removeFirst();
                    cnt++;
                    tiAndOr.two--;
                }
                while (cnt < 4 && tiAndOr.onetwo > 0) {
                    helpers.append(listOneTwo.first());
                    listOneTwo.removeFirst();
                    cnt++;
                    tiAndOr.onetwo--;
                }
                while (cnt < 4 && tiAndOr.self > 0) {
                    helpers.append(listAndOr.first());
                    listAndOr.removeFirst();
                    cnt++;
                    tiAndOr.self--;
                }

                JUEntity::MappedSignals ms = m_notMappedEntitySignals.first();
                m_notMappedEntitySignals.removeFirst();
                JUProtoUBS firstHelper = helpers.first();
                if (firstHelper.type() == JUProtoUBS::Two) {
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[0]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[1]);
                } else if (firstHelper.type() == JUProtoUBS::AndOr) {
                    firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                    firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                } else if (firstHelper.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = firstHelper.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "0");
                    } else if (ports.contains(2) && ports.contains(3)) {
                        firstHelper.mapPortToSignal(2, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(3, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "0");
                    }
                }
                helpers.removeFirst();
                JUProtoUBS second = helpers.first();
                if (second.type() == JUProtoUBS::Two) {
                    second.mapPortToSignal(second.getFirstAvailablePort(), ms.signalIn[2]);
                    second.mapPortToSignal(second.getFirstAvailablePort(), "0");
                } else if (second.type() == JUProtoUBS::AndOr) {
                    second.mapPortToSignal(0, ms.signalIn[2]);
                    second.mapPortToSignal(1, "0");
                    second.mapPortToSignal(second.getFirstAvailablePort(), "1");
                } else if (second.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = second.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        second.mapPortToSignal(0, "1");
                        second.mapPortToSignal(1, "1");
                        second.mapPortToSignal(second.getFirstAvailablePort(), ms.signalIn[2]);
                    } else if (ports.contains(2) && ports.contains(3)) {
                        second.mapPortToSignal(2, "1");
                        second.mapPortToSignal(3, "1");
                        second.mapPortToSignal(second.getFirstAvailablePort(), ms.signalIn[2]);
                    }
                }
                helpers.removeFirst();
                JUProtoUBS third = helpers.first();
                if (third.type() == JUProtoUBS::Two) {
                    third.mapPortToSignal(third.getFirstAvailablePort(), ms.signalIn[3]);
                    third.mapPortToSignal(third.getFirstAvailablePort(), "0");
                } else if (third.type() == JUProtoUBS::AndOr) {
                    third.mapPortToSignal(0, ms.signalIn[3]);
                    third.mapPortToSignal(1, "0");
                    third.mapPortToSignal(third.getFirstAvailablePort(), "1");
                } else if (third.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = third.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        third.mapPortToSignal(0, "1");
                        third.mapPortToSignal(1, "1");
                        third.mapPortToSignal(third.getFirstAvailablePort(), ms.signalIn[3]);
                    } else if (ports.contains(2) && ports.contains(3)) {
                        third.mapPortToSignal(2, "1");
                        third.mapPortToSignal(3, "1");
                        third.mapPortToSignal(third.getFirstAvailablePort(), ms.signalIn[3]);
                    }
                }
                helpers.removeFirst();

                andOr.mapPortToSignal(1, firstHelper.outputName());
                andOr.mapPortToSignal(0, second.outputName());
                andOr.mapPortToSignal(3, third.outputName());

                JUProtoUBS fourth = helpers.first();
                if (fourth.type() == JUProtoUBS::Two) {
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), andOr.outputName());
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), "0");
                } else if (fourth.type() == JUProtoUBS::AndOr) {
                    fourth.mapPortToSignal(0, andOr.outputName());
                    fourth.mapPortToSignal(1, "0");
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), "1");
                } else if (fourth.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = fourth.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        fourth.mapPortToSignal(0, "1");
                        fourth.mapPortToSignal(1, "1");
                        fourth.mapPortToSignal(fourth.getFirstAvailablePort(), andOr.outputName());
                    } else if (ports.contains(2) && ports.contains(3)) {
                        fourth.mapPortToSignal(2, "1");
                        fourth.mapPortToSignal(3, "1");
                        fourth.mapPortToSignal(fourth.getFirstAvailablePort(), andOr.outputName());
                    }
                }
                fourth.setOutputName(ms.signalOut[0]);
                helpers.removeFirst();
                m_mapped.append(firstHelper);
                m_mapped.append(second);
                m_mapped.append(third);
                m_mapped.append(fourth);
                m_mapped.append(andOr);
            }

            for (int i = 0; i < tiOneTwo.onetwo; ++i) {
                if (m_notMappedEntitySignals.count() == 0) break;

                JUProtoUBS oneTwo = listOneTwo.first();
                listAndOr.removeFirst();

                QList<JUProtoUBS> helpers;
                int cnt = 0;
                while (cnt < 2 && tiOneTwo.two > 0) {
                    helpers.append(listTwo.first());
                    listTwo.removeFirst();
                    cnt++;
                    tiOneTwo.two--;
                }
                while (cnt < 2 && tiOneTwo.andor > 0) {
                    helpers.append(listAndOr.first());
                    listAndOr.removeFirst();
                    cnt++;
                    tiOneTwo.andor--;
                }
                while (cnt < 2 && tiOneTwo.self > 0) {
                    helpers.append(listOneTwo.first());
                    listOneTwo.removeFirst();
                    cnt++;
                    tiOneTwo.self--;
                }

                JUEntity::MappedSignals ms = m_notMappedEntitySignals.first();
                m_notMappedEntitySignals.removeFirst();
                JUProtoUBS firstHelper = helpers.first();
                if (firstHelper.type() == JUProtoUBS::Two) {
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[0]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[1]);
                } else if (firstHelper.type() == JUProtoUBS::AndOr) {
                    firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                    firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                } else if (firstHelper.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = firstHelper.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                    } else if (ports.contains(2) && ports.contains(3)) {
                        firstHelper.mapPortToSignal(2, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(3, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                    }
                }
                helpers.removeFirst();
                JUProtoUBS second = helpers.first();
                if (second.type() == JUProtoUBS::Two) {
                    second.mapPortToSignal(second.getFirstAvailablePort(), firstHelper.outputName());
                    second.mapPortToSignal(second.getFirstAvailablePort(), "0");
                } else if (second.type() == JUProtoUBS::AndOr) {
                    second.mapPortToSignal(0, firstHelper.outputName());
                    second.mapPortToSignal(1, "0");
                    second.mapPortToSignal(second.getFirstAvailablePort(), "1");
                } else if (second.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = second.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        second.mapPortToSignal(0, "1");
                        second.mapPortToSignal(1, "1");
                        second.mapPortToSignal(second.getFirstAvailablePort(), firstHelper.outputName());
                    } else if (ports.contains(2) && ports.contains(3)) {
                        second.mapPortToSignal(2, "1");
                        second.mapPortToSignal(3, "1");
                        second.mapPortToSignal(second.getFirstAvailablePort(), firstHelper.outputName());
                    }
                }
                helpers.removeFirst();

                QList<int> avail = oneTwo.portsAvail();
                if (avail.contains(0) && avail.contains(1)) {
                    oneTwo.mapPortToSignal(0, ms.signalIn[2]);
                    oneTwo.mapPortToSignal(1, ms.signalIn[3]);
                    oneTwo.mapPortToSignal(oneTwo.getFirstAvailablePort(), second.outputName());
                } else {
                    oneTwo.mapPortToSignal(2, ms.signalIn[2]);
                    oneTwo.mapPortToSignal(3, ms.signalIn[3]);
                    oneTwo.mapPortToSignal(oneTwo.getFirstAvailablePort(), second.outputName());
                }
                oneTwo.setOutputName(ms.signalOut[0]);

                m_mapped.append(firstHelper);
                m_mapped.append(second);
                m_mapped.append(oneTwo);
            }

            while (m_notMappedEntitySignals.count() > 0) {
                JUEntity::MappedSignals ms = m_notMappedEntitySignals.first();
                m_notMappedEntitySignals.removeFirst();

                int cnt = 0;
                QList<JUProtoUBS> twos;
                while (cnt < 4 && listTwo.count() > 0) {
                    twos.append(listTwo.first());
                    listTwo.removeFirst();
                    cnt++;
                }
                while (cnt < 4 && listOneTwo.count() > 0) {
                    twos.append(listOneTwo.first());
                    listOneTwo.removeFirst();
                    cnt++;
                }
                while (cnt < 4 && listAndOr.count() > 0) {
                    twos.append(listAndOr.first());
                    listAndOr.removeFirst();
                    cnt++;
                }

                JUProtoUBS firstHelper = twos.first();
                if (firstHelper.type() == JUProtoUBS::Two) {
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[0]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), ms.signalIn[1]);
                } else if (firstHelper.type() == JUProtoUBS::AndOr) {
                    firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                    firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                    firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                } else if (firstHelper.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = firstHelper.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        firstHelper.mapPortToSignal(0, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(1, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                    } else if (ports.contains(2) && ports.contains(3)) {
                        firstHelper.mapPortToSignal(2, ms.signalIn[0]);
                        firstHelper.mapPortToSignal(3, ms.signalIn[1]);
                        firstHelper.mapPortToSignal(firstHelper.getFirstAvailablePort(), "1");
                    }
                }
                twos.removeFirst();
                JUProtoUBS second = twos.first();
                if (second.type() == JUProtoUBS::Two) {
                    second.mapPortToSignal(second.getFirstAvailablePort(), ms.signalIn[2]);
                    second.mapPortToSignal(second.getFirstAvailablePort(), ms.signalIn[3]);
                } else if (second.type() == JUProtoUBS::AndOr) {
                    second.mapPortToSignal(0, ms.signalIn[2]);
                    second.mapPortToSignal(1, ms.signalIn[3]);
                    second.mapPortToSignal(second.getFirstAvailablePort(), "1");
                } else if (second.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = second.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        second.mapPortToSignal(0, ms.signalIn[2]);
                        second.mapPortToSignal(1, ms.signalIn[3]);
                        second.mapPortToSignal(second.getFirstAvailablePort(), "1");
                    } else if (ports.contains(2) && ports.contains(3)) {
                        second.mapPortToSignal(2, ms.signalIn[2]);
                        second.mapPortToSignal(3, ms.signalIn[3]);
                        second.mapPortToSignal(second.getFirstAvailablePort(), "1");
                    }
                }
                twos.removeFirst();
                JUProtoUBS third = twos.first();
                if (third.type() == JUProtoUBS::Two) {
                    third.mapPortToSignal(third.getFirstAvailablePort(), firstHelper.outputName());
                    third.mapPortToSignal(third.getFirstAvailablePort(), second.outputName());
                } else if (third.type() == JUProtoUBS::AndOr) {
                    third.mapPortToSignal(0, firstHelper.outputName());
                    third.mapPortToSignal(1, second.outputName());
                    third.mapPortToSignal(third.getFirstAvailablePort(), "1");
                } else if (third.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = third.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        third.mapPortToSignal(0, firstHelper.outputName());
                        third.mapPortToSignal(1, second.outputName());
                        third.mapPortToSignal(third.getFirstAvailablePort(), "1");
                    } else if (ports.contains(2) && ports.contains(3)) {
                        third.mapPortToSignal(2, firstHelper.outputName());
                        third.mapPortToSignal(3, second.outputName());
                        third.mapPortToSignal(third.getFirstAvailablePort(), "1");
                    }
                }
                twos.removeFirst();

                JUProtoUBS fourth = twos.first();
                if (fourth.type() == JUProtoUBS::Two) {
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), third.outputName());
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), "0");
                } else if (fourth.type() == JUProtoUBS::AndOr) {
                    fourth.mapPortToSignal(0, third.outputName());
                    fourth.mapPortToSignal(1, "0");
                    fourth.mapPortToSignal(fourth.getFirstAvailablePort(), "1");
                } else if (fourth.type() == JUProtoUBS::OneTwo) {
                    QList<int> ports = fourth.portsAvail();
                    if (ports.contains(0) && ports.contains(1)) {
                        fourth.mapPortToSignal(0, "1");
                        fourth.mapPortToSignal(1, "1");
                        fourth.mapPortToSignal(fourth.getFirstAvailablePort(), third.outputName());
                    } else if (ports.contains(2) && ports.contains(3)) {
                        fourth.mapPortToSignal(2, "1");
                        fourth.mapPortToSignal(3, "1");
                        fourth.mapPortToSignal(fourth.getFirstAvailablePort(), third.outputName());
                    }
                }
                fourth.setOutputName(ms.signalOut[0]);

                m_mapped.append(firstHelper);
                m_mapped.append(second);
                m_mapped.append(third);
                m_mapped.append(fourth);
            }
        }
    }

    m_vhdl = vhdlDescriptionInner(&m_mapped);
    m_pixmap = pixmapDescriptionInner(&m_mapped);

    return true;
}

QString JUReconfUBS::vhdlDescriptionInner(QList<JUProtoUBS> *luts)
{
    QString vhdl = "";

    QString portsDesc = (*m_mainEntity).portsIn()[0].name;
    for (int i = 1; i < m_mainEntity->portsIn().count(); ++i) {
        portsDesc = portsDesc % ", " % (*m_mainEntity).portsIn()[i].name;
    }

    QString output = m_mainEntity->portsOut()[0].name;

    QString signalDesc = "";
    for (int i = 0; i < m_mapped.count(); ++i) {
        JUProtoUBS u = m_mapped[i];
        if (u.outputName() == output) {
            continue;
        }
        if (signalDesc.length() > 0) {
            signalDesc = signalDesc % ", ";
        }
        signalDesc = signalDesc % u.outputName();
    }

    vhdl = vhdl % "\nentity main_entity is\n";
    vhdl = vhdl % "    port (\n";
    vhdl = vhdl % "        " % portsDesc % ": in BIT;\n";
    vhdl = vhdl % "        " %  output % ": out BIT;\n";
    vhdl = vhdl % "    );\n";
    vhdl = vhdl % "end entity;\n\n";
    vhdl = vhdl % "architecture main_entity_arch of main_entity is\n";
        vhdl = vhdl % "    component 2band2_or2 port(tx1, tx2, tx3, tx4: in BIT; txout: out BIT);\n";
        vhdl = vhdl % "    end component;\n";
    if (signalDesc.length() > 0) {
        vhdl = vhdl % "\n    signal " % signalDesc % ": BIT;\n";
    }
    vhdl = vhdl % "begin\n";
    for (int i = 0; i < m_mapped.count(); ++i) {
        JUProtoUBS u = m_mapped[i];
        QString pd = "";
        for (int j = 0; j < 4; ++j) {
            if (j > 0) {
                pd = pd % ", ";
            }
            QString qs = u.portSignalMap()[j];
            if (qs.isEmpty()) {
                qs = "0";
            }
            pd = pd % qs;
        }
        vhdl = vhdl % "    " % u.name().toUpper() % ": 2band2_or2 port map(" % pd % ", " % u.outputName() % ");\n";
    }
    vhdl = vhdl % "end architecture main_entity_arch;";

    return vhdl;
}

QPixmap JUReconfUBS::pixmapDescriptionInner(QList<JUProtoUBS> *luts)
{
    QMap<int, QList<JUProtoUBS>> levels;
    QList<JUProtoUBS> copy;
    copy.append(*luts);

    int maxElemCount = 0;

    int level = 0;
    int used = 0;
    QSet<QString> currentLevelInputs;
    currentLevelInputs.insert("1");
    currentLevelInputs.insert("0");
    QSet<QString> nextLevelInputs;
    levels[0] = QList<JUProtoUBS>();
    while (copy.count() > 0) {
        QList<int> toremove;
        for (int i = 0; i < copy.count(); ++i) {
            JUProtoUBS l = copy[i];
            if (level == 0) {
                QSet<QString> ls = QSet<QString>::fromList(l.portSignalMap().values());
                QSet<QString> is;
                for (int hhh = 0; hhh < (*m_mainEntity).portsIn().count(); ++hhh) {
                    is.insert((*m_mainEntity).portsIn()[hhh].name);
                }
                is.insert("1");
                is.insert("0");
                if (ls.subtract(is).count() == 0) {
                    levels[0].append(l);
                    nextLevelInputs.insert(l.outputName());
                    toremove.append(i);
                }
            } else {
                QSet<QString> s = QSet<QString>::fromList(l.portSignalMap().values());
                if (s.subtract(currentLevelInputs).count() == 0) {
                    if (!levels.contains(level)) {
                        levels[level] = QList<JUProtoUBS>();
                    }
                    levels[level].append(l);
                    nextLevelInputs.insert(l.outputName());
                    toremove.append(i);
                }
            }
        }
        for (int i = toremove.count() - 1; i > -1; --i) {
            copy.removeAt(toremove[i]);
        }
        if (levels[level].count() > maxElemCount) {
            maxElemCount = levels[level].count();
        }
        level++;
        currentLevelInputs.unite(nextLevelInputs);
        nextLevelInputs.clear();
    }

    level++;
    int inputsCount = (*m_mainEntity).portsIn().count() + 2;
    int pixmapWidth = (level + 1) * LUT_ELEM_WIDTH + (level) * DISTANCE_BETWEEN_LEVELS_X + 2 * PIXMAP_MARGIN;
    int pixmapHeight = 2 * PIXMAP_MARGIN + qMax(maxElemCount * m_ubsElemHeight + (maxElemCount - 1) * DISTANCE_BETWEEN_LUTS_Y, inputsCount * INPUT_ELEM_HEIGHT + (inputsCount - 1) * DISTANCE_MIN_BETWEEN_INPUTS);
    QPixmap pixmap(pixmapWidth, pixmapHeight);
    QPainter p(&pixmap);
    p.fillRect(0, 0, pixmapWidth, pixmapHeight, Qt::white);

    int distanceBetweenInputs = (pixmapHeight - 2 * PIXMAP_MARGIN - inputsCount * INPUT_ELEM_HEIGHT) / (inputsCount - 1);

    QList<QString> inputs;
    for (int i = 0; i < (*m_mainEntity).portsIn().count(); ++i) {
        inputs.append((*m_mainEntity).portsIn()[i].name);
    }
    int x = PIXMAP_MARGIN;
    int y = PIXMAP_MARGIN;
    for (int i = 0; i < inputsCount - 2; ++i) {
        QString name = inputs[i];
        drawInput(x, y, name, &p);
        y += INPUT_ELEM_HEIGHT + distanceBetweenInputs;
    }
    drawInput(x, y, "0", &p);
    inputs.append("0");
    y += INPUT_ELEM_HEIGHT + distanceBetweenInputs;
    drawInput(x, y, "1", &p);
    inputs.append("1");

    x = PIXMAP_MARGIN + m_inputRects[inputs[0]].width() + DISTANCE_BETWEEN_LEVELS_X;
    for (int i = 0; i < level - 1; ++i) {
        QList<JUProtoUBS> ll = levels[i];
        int count = ll.count();

        int height = count * m_ubsElemHeight + (count - 1) * DISTANCE_BETWEEN_LUTS_Y;
        int y = PIXMAP_MARGIN + (pixmapHeight - 2 * PIXMAP_MARGIN - height) / 2;
        
        for (int j = 0; j < count; ++j) {
            JUProtoUBS l = ll[j];
            drawUBS(x, y, l, &p);
            y += m_ubsElemHeight + DISTANCE_BETWEEN_LUTS_Y;
        }

        x += LUT_ELEM_WIDTH + DISTANCE_BETWEEN_LEVELS_X;
    }

    for (int i = 0; i < inputs.count(); ++i) {
        QString input = inputs[i];

        p.save();
        QRect inputRect = m_inputRects[input];
        p.setPen(m_inputColors[input]);
        p.setBrush(m_inputColors[input]);
        int yStart = inputRect.y() + inputRect.height() / 2;

        int offsetForLineX = DISTANCE_BETWEEN_LINES * (i + 1);

        int skippedLevels = 0;

        for (int j = 0; j < level - 1; ++j) {
            QList<int> portsY;
            QList<JUProtoUBS> luts = levels[j];
            for (int k = 0; k < luts.count(); ++k) {
                JUProtoUBS l = luts[k];
                if (input == "0") {
                    QList<int> broken = l.brokenPorts();
                    for (int m = 0; m < broken.count(); ++m) {
                        QRect r = m_ubsRects[l.name()];
                        portsY.append(r.y() + LUT_ELEM_MARGIN + broken[m] * DISTANCE_BETWEEN_LUT_PORTS);
                    }
                    QList<int> unused = l.portsAvail();
                    for (int m = 0; m < unused.count(); ++m) {
                        QRect r = m_ubsRects[l.name()];
                        portsY.append(r.y() + LUT_ELEM_MARGIN + unused[m] * DISTANCE_BETWEEN_LUT_PORTS);
                    }
                    if (l.portSignalMap().values().contains("0")){
                        for (int ggg = 0; ggg < l.portSignalMap().count(); ++ggg) {
                            int key = l.portSignalMap().keys()[ggg];
                            if (l.portSignalMap()[key] == "0") {
                                QRect r = m_ubsRects[l.name()];
                                portsY.append(r.y() + LUT_ELEM_MARGIN + key * DISTANCE_BETWEEN_LUT_PORTS);
                            }
                        }
                    }
                } else if (input == "1") {
                    if (l.portSignalMap().values().contains("1")){
                        for (int ggg = 0; ggg < l.portSignalMap().count(); ++ggg) {
                            int key = l.portSignalMap().keys()[ggg];
                            if (l.portSignalMap()[key] == "1") {
                                QRect r = m_ubsRects[l.name()];
                                portsY.append(r.y() + LUT_ELEM_MARGIN + key * DISTANCE_BETWEEN_LUT_PORTS);
                            }
                        }
                    }
                } else if (l.portSignalMap().values().contains((*m_mainEntity).portsIn()[i].name)) {
                    QRect r = m_ubsRects[l.name()];
                    int signal = -1;
                    for (int jjj = 0; jjj < l.portSignalMap().count(); ++jjj) {
                        if (l.portSignalMap()[l.portSignalMap().keys()[jjj]] == input) {
                            signal = l.portSignalMap().keys()[jjj];
                            break;
                        }
                    }
                    portsY.append(r.y() + LUT_ELEM_MARGIN + signal * DISTANCE_BETWEEN_LUT_PORTS);
                }
            }

            if (portsY.count() == 0) {
                skippedLevels++;
                continue;
            }

            if (j > 0) {
                for (int m = skippedLevels; m > 0; --m) {
                    int lvl = j - m;
                    QList<JUProtoUBS> luts = levels[lvl];
                    int x = PIXMAP_MARGIN + inputRect.width() + lvl * DISTANCE_BETWEEN_LEVELS_X + lvl * LUT_ELEM_WIDTH;
                    int lineX = x + offsetForLineX;
                    QRect block(-1, -1, -1, -1);
                    for (int mm = 0; mm < luts.count(); ++mm) {
                        QRect r = m_ubsRects[luts[mm].name()];
                        if (r.top() <= yStart && r.top() + r.height() >= yStart) {
                            block = r;
                            break;
                        }
                    }
                    if (lvl == 0) {
                        p.drawLine(x, yStart, lineX, yStart);
                    }
                    if (block.width() > -1) {
                        int up = yStart - block.top();
                        int down = block.bottom() - yStart;
                        int offset = DISTANCE_BETWEEN_LUTS_Y * 1.0 / (10 * inputsCount) * (i + 1);
                        if (up < down) {
                            p.drawLine(lineX, yStart, lineX, block.top() - offset);
                            p.drawLine(lineX, block.top() - offset, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.top() - offset);
                            yStart = block.top() - offset;
                        } else if (up > down) {
                            p.drawLine(lineX, yStart, lineX, block.bottom() + offset);
                            p.drawLine(lineX, block.bottom() + offset, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.bottom() + offset);
                            yStart = block.bottom() + offset;
                        }
                    } else {
                        p.drawLine(lineX, yStart, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, yStart);
                    }
                }

                int lvl = j - 1;
                int x = PIXMAP_MARGIN + inputRect.width() + lvl * DISTANCE_BETWEEN_LEVELS_X + lvl * LUT_ELEM_WIDTH;
                int lineX = x + offsetForLineX;
                QList<JUProtoUBS> luts = levels[lvl];
                QRect block(-1, -1, -1, -1);
                for (int mm = 0; mm < luts.count(); ++mm) {
                    QRect r = m_ubsRects[luts[mm].name()];
                    if (r.top() <= yStart && r.top() + r.height() >= yStart) {
                        block = r;
                        break;
                    }
                }
                if (block.width() > -1) {
                    int up = yStart - block.top();
                    int down = block.bottom() - yStart;
                    int offset = DISTANCE_BETWEEN_LUTS_Y * 1.0 / (10 * inputsCount) * (i + 1);
                    if (up < down) {
                        p.drawLine(lineX, yStart, lineX, block.top() - offset);
                        p.drawLine(lineX, block.top() - offset, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.top() - offset);
                        yStart = block.top() - offset;
                    } else if (up > down) {
                        p.drawLine(lineX, yStart, lineX, block.bottom() + offset);
                        p.drawLine(lineX, block.bottom() + offset, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.bottom() + offset);
                        yStart = block.bottom() + offset;
                    }
                } else {
                    p.drawLine(lineX, yStart, lineX + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, yStart);
                }
            }

            portsY = sortListInt(portsY, 0, portsY.count() - 1);

            int x = PIXMAP_MARGIN + inputRect.width() + j * DISTANCE_BETWEEN_LEVELS_X + j * LUT_ELEM_WIDTH;
            int lineX = x + offsetForLineX;
            int portX = x + DISTANCE_BETWEEN_LEVELS_X;
            p.drawLine(lineX, portsY[0], lineX, portsY.last());
            for (int k = 0; k < portsY.count(); ++k) {
                p.drawLine(lineX, portsY[k], portX, portsY[k]);
                if (k > 0 && k < portsY.count() - 1) {
                    p.drawEllipse(QPoint(lineX, portsY[k]), CIRCLE_RADIUS, CIRCLE_RADIUS);
                }
            }
            p.drawLine(x, yStart, lineX, yStart);
            if (portsY.count() > 1 && yStart >= portsY[0] && yStart <= portsY.last()) {
                p.drawEllipse(QPoint(lineX, yStart), CIRCLE_RADIUS, CIRCLE_RADIUS);
            }
            if (yStart < portsY.first()) {
                p.drawLine(lineX, yStart, lineX, portsY.first());
                if (portsY.count() > 1) {
                    p.drawEllipse(QPoint(lineX, portsY.first()), CIRCLE_RADIUS, CIRCLE_RADIUS);
                }
            }
            if (yStart > portsY.last()) {
                p.drawLine(lineX, yStart, lineX, portsY.last());
                if (portsY.count() > 1) {
                    p.drawEllipse(QPoint(lineX, portsY.last()), CIRCLE_RADIUS, CIRCLE_RADIUS);
                }
            }

            skippedLevels = 0;
        }

        p.restore();
    }

    p.save();
    for (int i = 0; i < level - 2; ++i) {
        QList<JUProtoUBS> luts = levels[i];
        for (int j = 0; j < luts.count(); ++j) {
            p.setPen(QColor(qrand() % 256, qrand() % 256, qrand() % 256));

            JUProtoUBS l = luts[j];
            QRect r = m_ubsRects[l.name()];
            int x = r.right() + 5 + qrand() % 25;
            int y = r.y() + r.height() / 2;
            p.drawLine(r.right() + 1, y, x, y);

            QString output = l.outputName();
            for (int k = i + 1; k < level - 1; ++k) {
                QList<JUProtoUBS> kluts = levels[k];
                int targetPort = -1;
                QRect targetRect(-1, -1, -1, -1);
                for (int m = 0; m < kluts.count(); ++m) {
                    JUProtoUBS l2 = kluts[m];
                    if (l2.portSignalMap().values().contains(output)) {
                        int signal = -1;
                        for (int jjj = 0; jjj < l.portSignalMap().count(); ++jjj) {
                            if (l2.portSignalMap()[l2.portSignalMap().keys()[jjj]] == output) {
                                signal = l2.portSignalMap().keys()[jjj];
                                break;
                            }
                        }

                        targetPort = signal;
                        targetRect = m_ubsRects[l2.name()];
                        break;
                    }
                }

                if (targetPort > -1) {
                    int portY = targetRect.y() + LUT_ELEM_MARGIN + targetPort * DISTANCE_BETWEEN_LUT_PORTS;
                    p.drawLine(x, y, x, portY);
                    p.drawLine(x, portY, targetRect.x(), portY);
                    break;
                } else {
                    QRect block(-1, -1, -1, -1);
                    for (int mm = 0; mm < kluts.count(); ++mm) {
                        QRect r = m_ubsRects[kluts[mm].name()];
                        if (r.top() <= y && r.top() + r.height() >= y) {
                            block = r;
                            break;
                        }
                    }
                    if (block.width() > -1) {
                        int up = y - block.top();
                        int down = block.bottom() - y;
                        int offset = DISTANCE_BETWEEN_LUTS_Y * 1.0 / (10 * inputsCount) * (15 + qrand() % 25);
                        if (up < down) {
                            p.drawLine(x, y, x, block.top() - offset);
                            p.drawLine(x, block.top() - offset, x + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.top() - offset);
                            y = block.top() - offset;
                        } else if (up > down) {
                            p.drawLine(x, y, x, block.bottom() + offset);
                            p.drawLine(x, block.bottom() + offset,  + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, block.bottom() + offset);
                            y = block.bottom() + offset;
                        }
                    } else {
                        p.drawLine(x, y, x + DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH, y);
                        x += DISTANCE_BETWEEN_LEVELS_X + LUT_ELEM_WIDTH;
                    }
                }
            }
        }
    }
    p.restore();

    QRect lastElem = m_ubsRects[levels.last()[0].name()];
    x = (lastElem.right() + 1) + DISTANCE_BETWEEN_LEVELS_X;
    y = lastElem.y() + (lastElem.height() + 1 - INPUT_ELEM_HEIGHT) / 2;
    drawOutput(x, y, &p);

    p.setPen(m_inputColors["output"]);
    p.drawLine(lastElem.right() + 1, lastElem.y() + (lastElem.height() + 1) / 2, x, lastElem.y() + (lastElem.height() + 1) / 2);

    p.restore();

    p.end();
    return pixmap;
}

void JUReconfUBS::drawInput(int x, int y, QString name, QPainter *p)
{
    p->save();
    QColor color = QColor(qrand() % 256, qrand() % 256, qrand() % 256);
    m_inputColors[name] = color;
    p->setPen(color);
    p->drawLine(x, y, x + INPUT_ELEM_WIDTH, y);
    p->drawLine(x + INPUT_ELEM_WIDTH, y, x + INPUT_ELEM_WIDTH + INPUT_ELEM_HEIGHT / 2, y + INPUT_ELEM_HEIGHT / 2);
    p->drawLine(x + INPUT_ELEM_WIDTH + INPUT_ELEM_HEIGHT / 2, y + INPUT_ELEM_HEIGHT / 2, x + INPUT_ELEM_WIDTH, y + INPUT_ELEM_HEIGHT);
    p->drawLine(x + INPUT_ELEM_WIDTH, y + INPUT_ELEM_HEIGHT, x, y + INPUT_ELEM_HEIGHT);
    p->drawLine(x, y + INPUT_ELEM_HEIGHT, x, y);
    p->restore();

    m_inputRects[name] = QRect(x, y, INPUT_ELEM_WIDTH + INPUT_ELEM_HEIGHT / 2, INPUT_ELEM_HEIGHT);

    p->drawText(x, y, INPUT_ELEM_WIDTH, INPUT_ELEM_HEIGHT, Qt::AlignCenter, name);
}

void JUReconfUBS::drawOutput(int x, int y, QPainter *p)
{
    int width = INPUT_ELEM_WIDTH + 15;

    p->save();
    m_inputColors["output"] = QColor(qrand() % 256, qrand() % 256, qrand() % 256);
    p->setPen(m_inputColors["output"]);
    p->drawLine(x, y + INPUT_ELEM_HEIGHT / 2, x + INPUT_ELEM_HEIGHT / 2, y);
    p->drawLine(x + INPUT_ELEM_HEIGHT / 2, y, x + width, y);
    p->drawLine(x + width, y, x + width, y + INPUT_ELEM_HEIGHT);
    p->drawLine(x + width, y + INPUT_ELEM_HEIGHT, x + INPUT_ELEM_HEIGHT / 2, y + INPUT_ELEM_HEIGHT);
    p->drawLine(x + INPUT_ELEM_HEIGHT / 2, y + INPUT_ELEM_HEIGHT, x, y + INPUT_ELEM_HEIGHT / 2);
    p->restore();

    p->drawText(x, y, width + 3, INPUT_ELEM_HEIGHT - 3, Qt::AlignCenter, "output");
}

void JUReconfUBS::drawUBS(int x, int y, JUProtoUBS l, QPainter *p)
{
    QRect r(x, y, LUT_ELEM_WIDTH, m_ubsElemHeight);
    p->drawRect(r);
    //p->drawText(r, Qt::AlignCenter, l.name());

    p->drawLine(x + LUT_ELEM_WIDTH / 2, y, x + LUT_ELEM_WIDTH / 2, y + m_ubsElemHeight);
    p->drawLine(x, y + m_ubsElemHeight / 2, x + LUT_ELEM_WIDTH / 2, y + m_ubsElemHeight / 2);
    p->drawText(QRect(x, y + 5, LUT_ELEM_WIDTH / 2 - 5, m_ubsElemHeight / 2), Qt::AlignRight | Qt::AlignTop, "&");
    p->drawText(QRect(x, y + m_ubsElemHeight / 2 + 5, LUT_ELEM_WIDTH / 2 - 5, m_ubsElemHeight / 2), Qt::AlignRight | Qt::AlignTop, "&");
    p->drawText(QRect(x + LUT_ELEM_WIDTH / 2, y + 5, LUT_ELEM_WIDTH / 2 - 5, m_ubsElemHeight), Qt::AlignRight | Qt::AlignTop, "1");

    m_ubsRects[l.name()] = r;

    p->save();
    p->setPen(Qt::black);
    p->setBrush(Qt::white);
    for (int i = 0; i < 4; ++i) {
        int offset = LUT_ELEM_MARGIN + i * DISTANCE_BETWEEN_LUT_PORTS;
        p->drawEllipse(QPoint(x, y + offset), CIRCLE_RADIUS, CIRCLE_RADIUS);
    }
    p->restore();
    
    QList<int> brokenPorts = l.brokenPorts();
    for (int i = 0; i < brokenPorts.count(); ++i) {
        int port = brokenPorts[i];
        int offset = LUT_ELEM_MARGIN + port * DISTANCE_BETWEEN_LUT_PORTS;
        int crossCenterY = y + offset;
        p->save();
        p->setPen(QPen(Qt::red, 2));
        p->drawLine(x - CROSS_WIDTH / 2, crossCenterY - CROSS_HEIGHT / 2, x + CROSS_WIDTH / 2, crossCenterY + CROSS_HEIGHT / 2);
        p->drawLine(x - CROSS_WIDTH / 2, crossCenterY + CROSS_HEIGHT / 2, x + CROSS_WIDTH / 2, crossCenterY - CROSS_HEIGHT / 2);
        p->restore();
    }
}

QList<int> JUReconfUBS::sortListInt(QList<int> list, int low, int high)
{
    if (high - low + 1 < 2) {
        return list;
    }

    int index = low + (high - low)/2;
    int p = list[index];
    int l = low;
    int r = high;

    while (l <= r) {
        if (list[l] < p) {
            l++;
            continue;
        }
        if (list[r] > p) {
            r--;
            continue;
        }
        int t = list[l];
        list[l++] = list[r];
        list[r--] = t;
    }

    list = sortListInt(list, low, index - 1);
    sortListInt(list, index + 1, high);

    return list;
}