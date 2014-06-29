
#include "JUReconfLUT.h"
#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"
 
#include <QStringBuilder>

#define __JUMODULE__ "ReconfLUT"

JUReconfLUT::JUReconfLUT(QList<QString> cmnLut, QList<JUSchemeErrorLUT> errors, int elemsCount, int reservedElemsCount, int elemPortsCount)
{
    m_cmnLut = cmnLut;
    m_errors = errors;
    m_elemsCount = elemsCount;
    m_reservedElemsCount = reservedElemsCount;
    m_elemPortsCount = elemPortsCount;

    for (int i = 0; i < m_elemsCount; ++i) {
        JUAssert((i >= m_errors.count()) || (i < m_errors.count() && m_errors[i].type() != JUSchemeErrorLUT::LUTSchemeErrorTypeNone), "errors array can not contain empty errors");
        m_freeElems.append(JUProtoLUT("lut" % QString::number(i), m_elemPortsCount, (i < m_errors.count() ? m_errors[i] : JUSchemeErrorLUT()), QString("y%1").arg(i)));
    }

    for (int i = 0; i < m_cmnLut.count(); ++i) {
        m_disjuncts.append(JUReconfigFunctDisjunct(m_cmnLut[i]));
    }

    uniteDisjuncts();
}

JUReconfLUT::~JUReconfLUT()
{
}

// ========================================

bool JUReconfLUT::configure()
{
    bool first = true;
    while (m_disjuncts.count() > 0 && m_freeElems.count() > 0) {
        //JUMLog("free lut elements count %d.", m_freeElems.count());
        //JUMLog("disjuncts count %d.", m_disjuncts.count());

        bool exceeded = false;
        if (!first) {
            int oldMaxRand = maxDisjunctRank();
            QList<JUReconfigFunctDisjunct> oldD;
            oldD.append(m_disjuncts);
            uniteDisjuncts(true);
            if (maxDisjunctRank() > maxLUTAvailPorts()) {
                m_disjuncts.clear();
                m_disjuncts.append(oldD);
                exceeded = true;
            }
            //JUMLog("after union, disjuncts count: %d.", m_disjuncts.count());
        } else {
            first = false;
        }

        if (!exceeded && maxDisjunctRank() > maxLUTAvailPorts()) {
            JUMLog("max disjunct rang exceeds lut elem max available ports.");
            JUAssert(false, "wtf");
            return false;
        }

        if (exceeded || m_disjuncts.count() == 1) {
            int dIdx = -1;
            int maxRank = maxDisjunctRank();
            JUReconfigFunctDisjunct maxD = m_disjuncts[0];
            for (int i = 0; i < m_disjuncts.count(); ++i) {
                if (m_disjuncts[i].rank() == maxRank) {
                    maxD = m_disjuncts[i];
                    dIdx = i;
                    break;
                }
            }
            int min = INT_MAX;
            int idx = -1;
            JUProtoLUT p = m_freeElems[0];
            for (int i = 0; i < m_freeElems.count(); ++i) {
                if ((m_freeElems[i].availablePortsCount() - maxRank >= 0) && (m_freeElems[i].availablePortsCount() - maxRank < min)) {
                    min = m_freeElems[i].availablePortsCount() - maxRank;
                    p = m_freeElems[i];
                    idx = i;
                }
            }

            p.setDisjunct(maxD);

            m_disjuncts.removeAt(dIdx);
            m_freeElems.removeAt(idx);
            m_savedElems.append(p);
        } else {
            int* mt = kuhnAlgorithm();

            QList<int> usedElemsIndexes;
            QList<int> savedDisjunctsIndexes;
            for (int i = 0; i < m_freeElems.count(); ++i) {
                if (*(mt + i) == -1) {
                    continue;
                }
                JUProtoLUT p = m_freeElems[i];
                JUReconfigFunctDisjunct d = m_disjuncts[*(mt + i)];
                p.setDisjunct(d);
                m_savedElems.append(p);

                usedElemsIndexes.append(i);
                savedDisjunctsIndexes.append(*(mt + i));
            }

            delete mt;
            for (int i = usedElemsIndexes.count() - 1; i > -1; --i) {
                m_freeElems.removeAt(usedElemsIndexes[i]);
            }
            for (int i = savedDisjunctsIndexes.count() - 1; i > -1; --i) {
                m_disjuncts.removeAt(savedDisjunctsIndexes[i]);
            }
        }
    }

    if (m_disjuncts.count() > 0 && m_freeElems.count() == 0) {
        JUMLog("not enough LUT elements.");
        return false;
    }

    QList<JUProtoLUT> full;
    QList<JUProtoLUT> empty;

    QList<JUProtoLUT> finished;

    for (int i = 0; i < m_savedElems.count(); ++i) {
        if (m_savedElems[i].availablePortsCount() == 0) {
            full.append(m_savedElems[i]);
        } else {
            empty.append(m_savedElems[i]);
        }
    }
    empty.append(m_freeElems);
    empty = sort(empty, 0, empty.count() - 1);

    bool process = true;
    while (process) {
        JUProtoLUT e = empty[0];
        while (e.availablePortsCount() > 0 && full.count() > 0) {
            JUProtoLUT f = full[0];
            e.connectPortToSignal(e.getFirstAvailablePort(), f.outputName());
            full.removeFirst();
            finished.append(f);
        }
        if (e.availablePortsCount() == 0) {
            full.append(e);
            empty.removeFirst();
        } else {
            empty.removeFirst();
            empty.insert(0, e);
        }

        int ec = empty.count();
        int fc = full.count();

        if (ec == 0 && fc == 0) {
            JUAssert(false, "wtf?");
        }

        if (ec == 0 && fc == 1) {
            finished.append(full[0]);
            break;
        }

        if (ec == 0 && fc > 1) {
            JUMLog("have full, haven't empty.");
            return false;
        }

        if (ec > 0 && fc == 0) {
            process = false;
        }
    }

    if (!process) {
        while (empty.count() > 1 && empty[0].isAvailable() == false) {
            JUProtoLUT p = empty[0];
            JUProtoLUT p2 = empty[1];
            if (!p2.isAvailable()) {
                p2.connectPortToSignal(p2.getFirstAvailablePort(), p.outputName());
            }
            finished.append(p);
            empty.removeFirst();
        }
        if (!empty[0].isAvailable()) {
            finished.append(empty[0]);
        }
    }

    for (int i = 0; i < finished.count(); ++i) {
        JUMLog("%s", Q(finished[i].description()));
    }

    m_vhdl = vhdlDescriptionInner(&finished);
    m_pixmap = pixmapDescriptionInner(&finished);
    //JUMLog("%s", Q(m_vhdl));

    return true;
}

// ========================================

void JUReconfLUT::uniteDisjuncts(bool incrementRank)
{
    int disjunctsCount = m_disjuncts.count();
    if (disjunctsCount < 2) {
        return;
    }

    int minRankDifference = 0;
    if (incrementRank) {
        minRankDifference = INT_MAX;
        for (int i = 0; i < m_disjuncts.count() - 1; ++i) {
            for (int j = i + 1; j < m_disjuncts.count(); ++j) {
                int diff = qAbs(JUReconfigFunctDisjunct::union_rank(m_disjuncts[i], m_disjuncts[j]) - qMax(m_disjuncts[i].rank(), m_disjuncts[j].rank()));
                if (diff < minRankDifference) {
                    minRankDifference = diff;
                }
            }
        }
    }

    int processedDisjunctsCount = 0;
    int step = 0;
    while (processedDisjunctsCount < disjunctsCount) {
        JUReconfigFunctDisjunct d = m_disjuncts[0];
        m_disjuncts.removeFirst();
        QList<int> indexesToRemove;
        for (int i = 0; i < m_disjuncts.count() - step; ++i) {
            JUReconfigFunctDisjunct d2 = m_disjuncts[i];
            if (qAbs(JUReconfigFunctDisjunct::union_rank(d, d2) - qMax(d.rank(), d2.rank())) == minRankDifference) {
                d += d2;
                indexesToRemove.append(i);
            }
        }
        for (int i = indexesToRemove.count() - 1; i > -1 ; --i) {
            m_disjuncts.removeAt(indexesToRemove[i]);
        }
        processedDisjunctsCount += indexesToRemove.count() + 1;
        m_disjuncts.append(d);
        step++;
    }
}

int* JUReconfLUT::kuhnAlgorithm()
{
    int dc = m_disjuncts.count();

    QMap<int, QList<int>> candidates;
    for (int i = 0; i < dc; ++i) {
        candidates[i] = QList<int>();
    }
    for (int i = 0; i < dc; ++i) {
        JUReconfigFunctDisjunct d = m_disjuncts[i];
        for (int j = 0; j < m_freeElems.count(); ++j) {
            JUProtoLUT p = m_freeElems[j];
            if (d.rank() == p.availablePortsCount()) {
                candidates[i].append(j);
            }
        }
    }

    bool *used = (bool *)malloc(sizeof(bool) * dc);
    int *matching = (int *)malloc(sizeof(int) * m_freeElems.count());
    for (int i = 0; i < dc; ++i) {
        *(used + i) = false;
    }
    for (int i = 0; i < m_freeElems.count(); ++i) {
        *(matching + i) = -1;
    }

    for (int i = 0; i < dc; ++i) {
        *(used + i) = false;
        checkMatching(i, &candidates, used, matching);
    }

    delete used;
    return matching;
}

bool JUReconfLUT::checkMatching(int v, QMap<int, QList<int>> *candidates, bool *used, int *matching)
{
    if (*(used + v)) {
        return false;
    }

    QList<int> candidatesForV = (*candidates)[v];

    *(used + v) = true;
    for (int i = 0; i < candidatesForV.count(); ++i) {
        int to = candidatesForV[i];
        if (*(matching + to) == -1 || checkMatching(*(matching + to), candidates, used, matching)) {
            *(matching + to) = v;
            return true;
        }
    }
    return false;
}

int JUReconfLUT::maxDisjunctRank()
{
    int max = 0;
    for (int i = 0; i < m_disjuncts.count(); ++i) {
        if (m_disjuncts[i].rank() > max) {
            max = m_disjuncts[i].rank();
        }
    }
    return max;
}

int JUReconfLUT::maxLUTAvailPorts()
{
    int max = 0;
    for (int i = 0; i < m_freeElems.count(); ++i) {
        if (m_freeElems[i].availablePortsCount() > max) {
            max = m_freeElems[i].availablePortsCount();
        }
    }
    return max;
}

QList<JUProtoLUT> JUReconfLUT::sort(QList<JUProtoLUT> list, int low, int high)
{
    if (high - low < 2) {
        return list;
    }

    int index = low + (high - low)/2;
    int p = list[index].availablePortsCount();
    int l = low;
    int r = high;

    while (l <= r) {
        if (list[l].availablePortsCount() < p) {
            l++;
            continue;
        }
        if (list[r].availablePortsCount() > p) {
            r--;
            continue;
        }
        JUProtoLUT t = list[l];
        list[l++] = list[r];
        list[r--] = t;
    }

    list = sort(list, low, index - 1);
    sort(list, index + 1, high);

    return list;
}

QString JUReconfLUT::vhdlDescriptionInner(QList<JUProtoLUT> *luts)
{
    QMap<QString, int> map;
    for (int i = 0; i < luts->count(); ++i) {
        JUProtoLUT l = (*luts)[i];
        map[l.outputName()] = i;
    }

    QString vhdl = "";
    for (int i = 0; i < luts->count(); ++i) {
        vhdl = vhdl % (*luts)[i].vhdlDescription() % "\n\n";
    }

    QString portsDesc = "x0";
    for (int i = 1; i < m_cmnLut[0].length(); ++i) {
        portsDesc = portsDesc % ", x" % QString::number(i);
    }

    QString lutPortsDesc = "tx0";
    for (int i = 1; i < m_elemPortsCount; ++i) {
        lutPortsDesc = lutPortsDesc % ", tx" % QString::number(i);
    }

    QString signalsDesc = "";
    for (int i = 0; i < luts->count() - 1; ++i) {
        if (i != 0) {
            signalsDesc = signalsDesc % ", ";
        }
        signalsDesc = signalsDesc % "s" % QString::number(i);
    }

    vhdl = vhdl % "\nentity main_entity is\n";
    vhdl = vhdl % "    port (\n";
    vhdl = vhdl % "        " % portsDesc % ": in BIT;\n";
    vhdl = vhdl % "        output: out BIT;\n";
    vhdl = vhdl % "    );\n";
    vhdl = vhdl % "end entity;\n\n";
    vhdl = vhdl % "architecture main_entity_arch of main_entity is\n";
    for (int i = 0; i < luts->count(); ++i) {
        JUProtoLUT l = (*luts)[i];
        vhdl = vhdl % "    component " % l.name() % " port(" % lutPortsDesc % ": in BIT; txout: out BIT);\n";
        vhdl = vhdl % "    end component;\n";
    }
    if (signalsDesc.length() > 0) {
        vhdl = vhdl % "\n    signal " % signalsDesc % ": BIT;\n";
    }
    vhdl = vhdl % "begin\n";
    for (int i = 0; i < luts->count(); ++i) {
        JUProtoLUT l = (*luts)[i];
        vhdl = vhdl % "    " % l.name() % " port map(";
        for (int j = 0; j < m_elemPortsCount; ++j) {
            if (j > 0) {
                vhdl = vhdl % ", ";
            }
            if (l.brokenPorts().contains(j)) {
                vhdl = vhdl % "0";
            } else if (l.portSignalMap().contains(j)) {
                vhdl = vhdl % "x" % QString::number(l.portSignalMap()[j]);
            } else if (l.portNamedSignalMap().contains(j)) {
                int idx = map[l.portNamedSignalMap()[j]];
                vhdl = vhdl % "s" % QString::number(idx);
            } else {
                //JUAssert(false, "wtf?");
                vhdl = vhdl % "0";
            }
        }
        if (i < luts->count() - 1) {
            vhdl = vhdl % ", s" % QString::number(i) % ");\n";
        } else {
            vhdl = vhdl % ", output);\n";
        }
    }
    vhdl = vhdl % "end architecture main_entity_arch;";

    return vhdl;
}

QPixmap JUReconfLUT::pixmapDescriptionInner(QList<JUProtoLUT> *luts)
{
    return QPixmap();
}