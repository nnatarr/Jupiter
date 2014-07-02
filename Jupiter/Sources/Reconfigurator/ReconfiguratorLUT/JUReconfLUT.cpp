
#include "JUReconfLUT.h"
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

#define __JUMODULE__ "ReconfLUT"

JUReconfLUT::JUReconfLUT(QList<QString> cmnLut, QList<JUSchemeErrorLUT> errors, int elemsCount, int reservedElemsCount, int elemPortsCount)
{
    m_cmnLut = cmnLut;
    m_errors = errors;
    m_elemsCount = elemsCount;
    m_reservedElemsCount = reservedElemsCount;
    m_elemPortsCount = elemPortsCount;

    m_lutElemHeight = 25 * (m_elemPortsCount - 1) + 2 * LUT_ELEM_MARGIN;

    for (int i = 0; i < m_elemsCount + reservedElemsCount; ++i) {
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
            m_errorMsg = "max disjunct rang exceeds lut elem max available ports.";
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
        m_errorMsg = "not enough LUT elements.";
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
        if (e.availablePortsCount() > 0) {
            empty.removeFirst();
            empty.insert(0, e);
        } else {
            empty.removeFirst();
            full.append(e);
        }

        int ec = 0;
        for (int i = 0; i < empty.count(); ++i) {
            if (!empty[i].isAvailable()) {
                ec++;
            }
        }
        int fc = full.count();

        if (ec == 0 && fc == 0) {
            JUAssert(false, "wtf?");
        }

        if (ec == 0 && fc == 1) {
            finished.append(full[0]);
            break;
        }

        if (ec == 0 && fc > 1) {
            m_errorMsg = "not enough LUT elements.";
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
    if (high - low + 1 < 2) {
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
    QMap<int, QList<JUProtoLUT>> levels;
    QList<JUProtoLUT> copy;
    copy.append(*luts);

    int maxElemCount = 0;

    int level = 0;
    int used = 0;
    QSet<QString> currentLevelInputs;
    QSet<QString> nextLevelInputs;
    levels[0] = QList<JUProtoLUT>();
    while (copy.count() > 0) {
        QList<int> toremove;
        for (int i = 0; i < copy.count(); ++i) {
            JUProtoLUT l = copy[i];
            if (l.isAvailable()) {
                toremove.append(i);
                continue;
            }
            if (level == 0) {
                if (l.portNamedSignalMap().count() == 0) {
                    levels[0].append(l);
                    nextLevelInputs.insert(l.outputName());
                    toremove.append(i);
                }
            } else {
                QSet<QString> s = QSet<QString>::fromList(l.portNamedSignalMap().values());
                if (s.subtract(currentLevelInputs).count() == 0) {
                    if (!levels.contains(level)) {
                        levels[level] = QList<JUProtoLUT>();
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
    int inputsCount = m_cmnLut[0].length() + 1;
    int pixmapWidth = (level + 1) * LUT_ELEM_WIDTH + (level) * DISTANCE_BETWEEN_LEVELS_X + 2 * PIXMAP_MARGIN;
    int pixmapHeight = 2 * PIXMAP_MARGIN + qMax(maxElemCount * m_lutElemHeight + (maxElemCount - 1) * DISTANCE_BETWEEN_LUTS_Y, inputsCount * INPUT_ELEM_HEIGHT + (inputsCount - 1) * DISTANCE_MIN_BETWEEN_INPUTS);
    QPixmap pixmap(pixmapWidth, pixmapHeight);
    QPainter p(&pixmap);
    p.fillRect(0, 0, pixmapWidth, pixmapHeight, Qt::white);

    int distanceBetweenInputs = (pixmapHeight - 2 * PIXMAP_MARGIN - inputsCount * INPUT_ELEM_HEIGHT) / (inputsCount - 1);

    QList<QString> inputs;
    int x = PIXMAP_MARGIN;
    int y = PIXMAP_MARGIN;
    for (int i = 0; i < inputsCount - 1; ++i) {
        QString name = "x" % QString::number(i);
        drawInput(x, y, name, &p);
        inputs.append(name);
        y += INPUT_ELEM_HEIGHT + distanceBetweenInputs;
    }
    drawInput(x, y, "0", &p);
    inputs.append("0");

    x = PIXMAP_MARGIN + m_inputRects["x0"].width() + DISTANCE_BETWEEN_LEVELS_X;
    for (int i = 0; i < level - 1; ++i) {
        QList<JUProtoLUT> ll = levels[i];
        int count = ll.count();

        int height = count * m_lutElemHeight + (count - 1) * DISTANCE_BETWEEN_LUTS_Y;
        int y = PIXMAP_MARGIN + (pixmapHeight - 2 * PIXMAP_MARGIN - height) / 2;
        
        for (int j = 0; j < count; ++j) {
            JUProtoLUT l = ll[j];
            drawLUT(x, y, l, &p);
            y += m_lutElemHeight + DISTANCE_BETWEEN_LUTS_Y;
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
            QList<JUProtoLUT> luts = levels[j];
            for (int k = 0; k < luts.count(); ++k) {
                JUProtoLUT l = luts[k];
                if (l.signalPortMap().contains(i)) {
                    QRect r = m_lutRects[l.name()];
                    portsY.append(r.y() + LUT_ELEM_MARGIN + l.signalPortMap()[i] * DISTANCE_BETWEEN_LUT_PORTS);
                } else if (input == "0") {
                    QList<int> broken = l.brokenPorts().values();
                    for (int m = 0; m < broken.count(); ++m) {
                        QRect r = m_lutRects[l.name()];
                        portsY.append(r.y() + LUT_ELEM_MARGIN + broken[m] * DISTANCE_BETWEEN_LUT_PORTS);
                    }
                    QList<int> unused = l.availablePorts();
                    for (int m = 0; m < unused.count(); ++m) {
                        QRect r = m_lutRects[l.name()];
                        portsY.append(r.y() + LUT_ELEM_MARGIN + unused[m] * DISTANCE_BETWEEN_LUT_PORTS);
                    }
                }
            }

            if (portsY.count() == 0) {
                skippedLevels++;
                continue;
            }

            if (j > 0) {
                for (int m = skippedLevels; m > 0; --m) {
                    int lvl = j - m;
                    QList<JUProtoLUT> luts = levels[lvl];
                    int x = PIXMAP_MARGIN + inputRect.width() + lvl * DISTANCE_BETWEEN_LEVELS_X + lvl * LUT_ELEM_WIDTH;
                    int lineX = x + offsetForLineX;
                    QRect block(-1, -1, -1, -1);
                    for (int mm = 0; mm < luts.count(); ++mm) {
                        QRect r = m_lutRects[luts[mm].name()];
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
                QList<JUProtoLUT> luts = levels[lvl];
                QRect block(-1, -1, -1, -1);
                for (int mm = 0; mm < luts.count(); ++mm) {
                    QRect r = m_lutRects[luts[mm].name()];
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
        QList<JUProtoLUT> luts = levels[i];
        for (int j = 0; j < luts.count(); ++j) {
            p.setPen(QColor(qrand() % 256, qrand() % 256, qrand() % 256));

            JUProtoLUT l = luts[j];
            QRect r = m_lutRects[l.name()];
            int x = r.right() + 5 + qrand() % 25;
            int y = r.y() + r.height() / 2;
            p.drawLine(r.right() + 1, y, x, y);

            QString output = l.outputName();
            for (int k = i + 1; k < level - 1; ++k) {
                QList<JUProtoLUT> kluts = levels[k];
                int targetPort = -1;
                QRect targetRect(-1, -1, -1, -1);
                for (int m = 0; m < kluts.count(); ++m) {
                    JUProtoLUT l2 = kluts[m];
                    if (l2.namedSignalPortMap().contains(output)) {
                        targetPort = l2.namedSignalPortMap()[output];
                        targetRect = m_lutRects[l2.name()];
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
                        QRect r = m_lutRects[kluts[mm].name()];
                        if (r.top() <= y && r.top() + r.height() >= y) {
                            block = r;
                            break;
                        }
                    }
                    if (block.width() > -1) {
                        int up = y - block.top();
                        int down = block.bottom() - y;
                        int offset = DISTANCE_BETWEEN_LUTS_Y * 1.0 / (10 * inputsCount) * (5 + qrand() % 25);
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
                    }
                }
            }
        }
    }
    p.restore();

    QRect lastElem = m_lutRects[levels.last()[0].name()];
    x = (lastElem.right() + 1) + DISTANCE_BETWEEN_LEVELS_X;
    y = lastElem.y() + (lastElem.height() + 1 - INPUT_ELEM_HEIGHT) / 2;
    drawOutput(x, y, &p);

    p.setPen(m_inputColors["output"]);
    p.drawLine(lastElem.right() + 1, lastElem.y() + (lastElem.height() + 1) / 2, x, lastElem.y() + (lastElem.height() + 1) / 2);

    p.restore();

    p.end();
    return pixmap;
}

void JUReconfLUT::drawInput(int x, int y, QString name, QPainter *p)
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

void JUReconfLUT::drawOutput(int x, int y, QPainter *p)
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

void JUReconfLUT::drawLUT(int x, int y, JUProtoLUT l, QPainter *p)
{
    QRect r(x, y, LUT_ELEM_WIDTH, m_lutElemHeight);
    p->drawRect(r);
    p->drawText(r, Qt::AlignCenter, l.name());

    m_lutRects[l.name()] = r;

    QList<int> brokenPorts = l.brokenPorts().values();
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

QList<int> JUReconfLUT::sortListInt(QList<int> list, int low, int high)
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