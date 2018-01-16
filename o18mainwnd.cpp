#include "o18mainwnd.h"
#include "ui_o18mainwnd.h"

#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include <algorithm>
#include <memory>

#include "mtdsetupdlg.h"

static const QColor CLRO18 = QColor(50, 150, 70);
static const QColor CLRO18B = QColor(140, 255, 120);
static const QColor CLRO17 = QColor(1, 92, 191);
static const QColor CLRO17B = QColor(1, 191, 255);
static const QColor CLRVOLAPP = QColor(191, 92, 1);
static const QColor CLRVOLAPPB = QColor(255, 191, 1);
static const QColor CLRPLOTTEXT = QColor(255, 255, 255, 180);

static const int defPlotBase = 517;
static const int defPlotLeft = 89;
static const int defPlotWidth = 114;
static const int defPlotStep = 126;
static const int defPlotHeight = 271;
static const int defPlotDHeight = 11;

#if defined(Q_OS_WIN)
#include <ShlObj.h>
#endif

static inline const QString getStdAppDataPath(const QString& projectName)
{
        static QString ret;
        if( ret.isEmpty() )
        {
                ret = projectName;
#ifdef Q_OS_WIN
                std::wstring::value_type path[MAX_PATH];
                if( SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path) == S_OK )
                        ret = QString().fromStdWString(path);
                else if( GetModuleFileName(NULL, path, MAX_PATH) != 0 )
                {
                        ret = QDir(QString().fromStdWString(path)).absolutePath();
                }
                else
                        return ret;
                if( !projectName.isEmpty() )
                        ret += QString(1, '\\') + projectName;
#else
                if( QDir( "/var/" ).exists() )
                {
                        ret = "/var/" + projectName;
                }
#endif
                if( !ret.isEmpty() && ret.right(1) != "/" && ret.right(1) != "\\" )
                        ret += "\\";
        }
        return ret;
}

O18MainWnd::O18MainWnd(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::O18MainWnd)
        , m_showPositioning(false)
        , m_curStep(-1)
        , m_bClosing(false)
        , m_resPlot(*new QCustomPlot)
        , m_resPlotTitle(0)
{
        ui->setupUi(this);

        m_resPlot.setParent(ui->centralwidget);
        m_resPlot.setVisible(false);

        ui->tabWidget->setCurrentIndex(TAB_SETTINGS);

        QDir appdir = getStdAppDataPath(QString("o18model"));;
        if( !appdir.exists() )
                appdir.mkpath(appdir.absolutePath());

//#ifdef Q_NO_DEBUG
        //m_iniFileName =  appdir.absolutePath() + "/o18model.ini";
//#else
        //m_iniFileName =  QApplication::applicationDirPath() + "/o18model.ini";
        m_iniFileName = BASE_DIR + "/o18model.ini";
//#endif

        for(int i = 0; i != QApplication::arguments().size(); ++i)
        {
                QString arg = QApplication::arguments().at(i).toLower();
                if( arg == "-p" || arg == "--position" )
                        m_showPositioning = true;
        }

        m_imagenes["begin"] = QPixmap(":/cols/col9-all-70");
        m_imagenes["col1-40"] = QPixmap(":/cols/col1-40");
        m_imagenes["col1-hl-40"] = QPixmap(":/cols/col1-hl-40");
        m_imagenes["col1-all-40"] = QPixmap(":/cols/col1-all-40");
        m_imagenes["col2-40"] = QPixmap(":/cols/col2-40");
        m_imagenes["col2-hl-40"] = QPixmap(":/cols/col2-hl-40");
        m_imagenes["col2-all-40"] = QPixmap(":/cols/col2-all-40");
        m_imagenes["col3-40"] = QPixmap(":/cols/col3-40");
        m_imagenes["col3-hl-40"] = QPixmap(":/cols/col3-hl-40");
        m_imagenes["col3-all-40"] = QPixmap(":/cols/col3-all-40");
        m_imagenes["col4-50"] = QPixmap(":/cols/col4-50");
        m_imagenes["col4-hl-50"] = QPixmap(":/cols/col4-hl-50");
        m_imagenes["col4-all-50"] = QPixmap(":/cols/col4-all-50");
        m_imagenes["col5-50"] = QPixmap(":/cols/col5-50");
        m_imagenes["col5-hl-50"] = QPixmap(":/cols/col5-hl-50");
        m_imagenes["col5-all-50"] = QPixmap(":/cols/col5-all-50");
        m_imagenes["col6-50"] = QPixmap(":/cols/col6-50");
        m_imagenes["col6-hl-50"] = QPixmap(":/cols/col6-hl-50");
        m_imagenes["col6-all-50"] = QPixmap(":/cols/col6-all-50");
        m_imagenes["col7-60"] = QPixmap(":/cols/col7-60");
        m_imagenes["col7-hl-60"] = QPixmap(":/cols/col7-hl-60");
        m_imagenes["col7-all-60"] = QPixmap(":/cols/col7-all-60");
        m_imagenes["col8-60"] = QPixmap(":/cols/col8-60");
        m_imagenes["col8-hl-60"] = QPixmap(":/cols/col8-hl-60");
        m_imagenes["col8-all-70"] = QPixmap(":/cols/col8-all-70");
        m_imagenes["col9-70"] = QPixmap(":/cols/col9-70");
        m_imagenes["col9-hl-70"] = QPixmap(":/cols/col9-hl-70");
        m_imagenes["col9-all-70"] = QPixmap(":/cols/col9-all-70");

        loadChartGeometry();

        for(int i = 0; i != COLCOUNT; ++i)
        {
                QCustomPlot* plot = new QCustomPlot(ui->wgtImage);
                setupBarChart(*plot, i);
                m_plots.push_back(plot);
                adjustPlotPosition(m_curStep, i, false);
                m_tables.push_back( this->findChild<QTableWidget*>(QString("tblCol%1").arg(i)) );

                QHeaderView& hv = *m_tables[i]->horizontalHeader();
                for( int j = 0, cnt = hv.count(); j != cnt; ++j)
                        hv.setSectionResizeMode(j, QHeaderView::ResizeToContents);
        }

        ui->wgtTables->setVisible(false);
        ui->tblVolApp->setVisible(false);

        QHeaderView& hv = *ui->tblVolApp->horizontalHeader();
        for( int j = 0, cnt = hv.count(); j != cnt; ++j)
                hv.setSectionResizeMode(j, QHeaderView::ResizeToContents);
        connect(&m_demoTimer, SIGNAL(timeout()), SLOT(onDemoTimer()));

        QPixmap& pix = m_imagenes["begin"];
        ui->wgtImage->setMinimumSize(pix.size());
        ui->wgtImage->setMaximumSize(pix.size());

        ui->lblImage->setPixmap(pix);
        ui->lblImage->setGeometry(0, 0, pix.width(), pix.height());

        ui->lblDemo->setGeometry(0, 0, pix.width(), pix.height());
        ui->lblDemo->setStyleSheet("background-color: rgb(0,0,0);");

        QLinearGradient alphaGradient(0, 0, pix.width(), 0);
        alphaGradient.setColorAt(0.0, Qt::transparent);
        alphaGradient.setColorAt(1.0, Qt::black);
        makeOpacity(ui->lblDemo, alphaGradient);

        ui->lblDemo->lower();
        ui->lblImage->lower();

        makeOpacity(ui->tabWidget, 0.5);
        ui->tbLog->setVisible(false);

        m_demoTimer.start(100);

        if( !m_showPositioning )
        {
                ui->wgtDebug->deleteLater();
        }
        else
        {
                ChartGeometry2UI();

                connect(ui->sbHeight, SIGNAL(valueChanged(int)), SLOT(onDebugPosChanged(int)));
                connect(ui->sbLeft, SIGNAL(valueChanged(int)), SLOT(onDebugPosChanged(int)));
                connect(ui->sbTop, SIGNAL(valueChanged(int)), SLOT(onDebugPosChanged(int)));
                connect(ui->sbWidth, SIGNAL(valueChanged(int)), SLOT(onDebugPosChanged(int)));
                connect(ui->sbChartNum, SIGNAL(valueChanged(int)), SLOT(onDebugChartChanged(int)));
                connect(ui->sbStepNum, SIGNAL(valueChanged(int)), SLOT(onDebugStepChanged(int)));
                connect(ui->cbVisible, SIGNAL(toggled(bool)), SLOT(onDebugVisibleChanged(bool)));
                connect(ui->cbDebugFinished, SIGNAL(toggled(bool)), SLOT(onDebugFinishedChanged(bool)));
                connect(ui->btnPosSave, SIGNAL(released()), SLOT(onDebugPosSave()));
                connect(ui->btnAddStep, SIGNAL(released()), SLOT(onBtnAddStepClicked()));

                ui->btnStartCalc->deleteLater();
                ui->tbLog->deleteLater();
        }

        QSettings sets(m_iniFileName, QSettings::IniFormat);
        m_exportDir = sets.value("export_dir", QString()).toString();
        m_model.parameters_load(sets);
        int sides = sets.value("sides_width", 250).toInt();
        sets.setValue("sides_width", sides);
        if( sides >= 0 )
        {
                ui->wgtLeft->setMinimumWidth(sides);
                ui->wgtLeft->setMaximumWidth(sides);
                ui->wgtRight->setMinimumWidth(sides);
                ui->wgtRight->setMaximumWidth(sides);
        }
        ui->TestLabel->setText(BASE_DIR);
        ui->cbFormulaType->setCurrentIndex(m_model.formula_type());

        double resPlotOp  = sets.value("ResPlotOpacity", 0.8).toDouble();
        if( resPlotOp < 0.001 || resPlotOp > 1. )
                resPlotOp = 0.8;
        sets.setValue("ResPlotOpacity", resPlotOp);
        makeOpacity(m_resPlot, resPlotOp);

        ui->btnStartCalc->setEnabled( !m_model.mtdevices().empty() );

        QStringList mtdnames = m_model.mtdevices_names();
        if( !mtdnames.empty() )
        {
                ui->cbFirstColMTD->addItems(mtdnames);
                ui->cbIntermColMTD->addItems(mtdnames);
                ui->cbLastColMTD->addItems(mtdnames);
                ui->cbFirstColMTD->setCurrentIndex(m_model.first_mtd());
                ui->cbIntermColMTD->setCurrentIndex(m_model.interm_mtd());
                ui->cbLastColMTD->setCurrentIndex(m_model.last_mtd());
                ui->sbTargetOutput->setValue( m_model.target_output() + 0.5);
                ui->sbInitialHeight->setValue(m_model.initial_height() + 0.5);
                ui->sbInitialPressure->setValue(m_model.initial_pressure() + 0.5);
                ui->sbInitialO18Conc->setValue(m_model.o18_initial_concentration());
                ui->sbInitialO17Conc->setValue(m_model.o17_initial_concentration());
                ui->sbStopO18Conc->setValue(m_model.o18_stop_last_column());
                ui->sbThetaIntermColumn->setValue(m_model.theta_interm() + 0.5);
                ui->sbThetaBegin->setValue(m_model.theta_1st_start() + 0.5);
                ui->sbThetaEnd->setValue(m_model.theta_1st_end() + 0.5);
                ui->sbThetaStep->setValue(m_model.theta_1st_step());
        }

        connect(ui->cbResO17, SIGNAL(toggled(bool)), SLOT(onCBResO1718_toggled(bool)));
        connect(ui->cbResO18, SIGNAL(toggled(bool)), SLOT(onCBResO1718_toggled(bool)));
        connect(&m_model, SIGNAL(prepare_to_step(int,int)), SLOT(model_on_prepare_to_step(int,int)));
        connect(&m_model, SIGNAL(substep_processed(int,int,int)), SLOT(model_on_substep_processed(int,int,int)));
        connect(&m_model, SIGNAL(step_processed(int,int)), SLOT(model_on_step_processed(int,int)));
        connect(&m_model, SIGNAL(theta_finished(int)), SLOT(model_theta_finished(int)));
        connect(&m_model, SIGNAL(finished()), SLOT(model_on_finished()));
        connect(&m_model, SIGNAL(fatal_error(QString)), SLOT(model_on_fatal_error(QString)));

        connect(ui->btnExit, SIGNAL(released()), SLOT(close()));
}

O18MainWnd::~O18MainWnd()
{
        delete ui;
}

void O18MainWnd::onDebugPosChanged(int)
{
        int chart = ui->sbChartNum->value();
        UI2ChartGeometry(chart);
}

void O18MainWnd::onDebugVisibleChanged(bool)
{
        int chart = ui->sbChartNum->value();
        UI2ChartGeometry(chart);
}

void O18MainWnd::onDebugStepChanged(int)
{
        int step = ui->sbStepNum->value();
        if( step + 1 >= m_chartsgeomery.size() )
        {
                ui->sbStepNum->setValue(step - 1);
                return;
        }
        m_curStep = step;
        ChartGeometry2UI();
}

void O18MainWnd::onDebugChartChanged(int)
{
        int chart = ui->sbChartNum->value();
        if( chart < 0 )
        {
                ui->sbChartNum->setValue(m_plots.size() - 1);
                return;
        }
        if( chart >= m_plots.size() )
        {
                ui->sbChartNum->setValue(0);
                return;
        }
        ChartGeometry2UI();
}

void O18MainWnd::onDebugPosSave()
{
        saveChartGeometry();
}

void O18MainWnd::onDebugFinishedChanged(bool)
{
        ChartGeometry2UI();
}

void O18MainWnd::onBtnAddStepClicked()
{
        m_chartsgeomery.resize(m_chartsgeomery.size() + 1);
        m_chartsgeomery[m_chartsgeomery.size() - 1] = m_chartsgeomery[m_chartsgeomery.size() - 2];
}

void O18MainWnd::onDemoTimer()
{
        int iplot = qrand() % m_plots.size();
        QCustomPlot* plot = m_plots[iplot];
        QCPBars* o18 = qobject_cast<QCPBars*>(plot->plottable(0));
        QCPBars* o17 = qobject_cast<QCPBars*>(plot->plottable(1));
        if( o18 != 0 && o17 != 0 )
        {
                int sz = o18->data()->size();
                std::vector<int> values(sz);
                for(int i = 0; i != sz; ++i)
                        values[i] = i;
                std::random_shuffle(values.begin(), values.end());
                int count = ((double)qrand()) / RAND_MAX * sz + .5;
                if( count  > sz ) count = sz;
                for(int i = 0; i != count; ++i)
                {
                        int index = values[i];
                        double dt = random() * 100.;
                        o18->removeData(index);
                        o18->addData(index, dt - dt/2);
                        o17->removeData(index);
                        o17->addData(index, dt / 2.);
                }
                plot->replot();
        }
}

void O18MainWnd::on_btnMTDSetup_clicked()
{
        MTDSetupDlg dlg(this, m_model.mtdevices());
        if( dlg.exec() )
        {
                mtdevices_t newlist = dlg.devices();
                if( newlist != m_model.mtdevices() )
                {
                        m_model.mtdevices_update(newlist);
                        QSettings sets(m_iniFileName, QSettings::IniFormat);
                        m_model.parameters_save(sets, true);
                        sets.sync();
                        ui->btnStartCalc->setEnabled( !m_model.mtdevices().empty() );
                        ui->cbFirstColMTD->clear();
                        ui->cbIntermColMTD->clear();
                        ui->cbLastColMTD->clear();
                        QStringList mtdnames = m_model.mtdevices_names();
                        if( !mtdnames.empty() )
                        {
                                ui->cbFirstColMTD->addItems(mtdnames);
                                ui->cbIntermColMTD->addItems(mtdnames);
                                ui->cbLastColMTD->addItems(mtdnames);
                                m_model.parameters_load(sets);
                                ui->cbFirstColMTD->setCurrentIndex(m_model.first_mtd());
                                ui->cbIntermColMTD->setCurrentIndex(m_model.interm_mtd());
                                ui->cbLastColMTD->setCurrentIndex(m_model.last_mtd());
                        }
                }
        }
}

void O18MainWnd::on_btnStartCalc_clicked()
{
        if( m_model.state() == ST_BEGIN || m_model.state() == ST_FINISH )
                start();
        else
                abort();
}

void O18MainWnd::on_btnDebugFinCopy_clicked()
{
        int chart = ui->sbChartNum->value();
        ChartCtrl& cc = m_chartsgeomery[m_curStep + 1][chart];
        cc.geometry_finished = cc.geometry;
        ChartGeometry2UI();
}

void O18MainWnd::on_btnCopyPrev_clicked()
{
        if( m_curStep !=  -1)
        {
                int stepFrom = m_curStep;
                int stepTo = m_curStep + 1;
                for(int i = 1; i != m_chartsgeomery[stepTo].size(); ++i)
                        m_chartsgeomery[stepTo][i] = m_chartsgeomery[stepFrom][i-1];
                ChartGeometry2UI();
        }
}

void O18MainWnd::on_btnExportResult_clicked()
{
        QFileDialog dl(this, tr("Export calculation data"));
        dl.setAcceptMode(QFileDialog::AcceptSave);
        if( !m_exportDir.isEmpty() )
                dl.setDirectory(m_exportDir);
        dl.selectFile("o18_calculation_results.csv");
        dl.setNameFilter(tr("CSV files (*.csv)"));
        if( dl.exec() )
                saveToFile(dl.selectedFiles().at(0) );
}

void O18MainWnd::on_cbResTheta_currentIndexChanged(int index)
{
        if( m_model.state() != ST_FINISH )
                return;
        for(int i = 0; i != m_tables.size(); ++i)
        {
                QTableWidget& tbl = *m_tables[i];
                clearTable(tbl);
                tbl.setVisible(false);
        }
        ui->wgtTables->setVisible(false);
        ui->tblVolApp->setVisible(false);
        if( ui->cbResTheta->count() > 1 && index == 0 )
        {
                //build vol.app. deprendence
                drawVolApp();
                return;
        }
        --index;
        if(index < 0 || index >= m_model.result().thetas.size() )
        {
                m_resPlot.setVisible(false);
                return;
        }
        const COneThetaResult& otr = m_model.result().result[index];
        ui->cbResPlotCol->blockSignals(true);
        ui->cbResPlotCol->clear();
        ui->cbResPlotCol->addItem(tr("None"));
        for(int i = 0; i != otr.colresults.size(); ++i)
        {
                const COneColumnResult& colres = otr.colresults[i];
                QTableWidget& tbl = *m_tables[i];
                tbl.setRowCount(colres.levels_count);
                for(int substep = 0; substep != colres.levels_count; ++substep)
                {
                        setTableValue(tbl, substep, COL_Z, tr("%1").arg(substep + 1));
                        setTableValue(tbl, substep, COL_ALPHA, tr("%1").arg(colres.alphas[substep],
                                                                                                                                0, 'f', -1));
                        setTableValue(tbl, substep, COL_O18, tr("%1").arg(colres.o18concentrations[substep],
                                                                                                                                0, 'f', -1));
                        setTableValue(tbl, substep, COL_O17, tr("%1").arg(colres.o17concentrations[substep],
                                                                                                                                0, 'f', -1));
                }
                ui->cbResPlotCol->addItem(tr("Column %1").arg(i + 1));
                tbl.setVisible(true);
        }
        ui->cbResPlotCol->blockSignals(false);
        ui->wgtTables->setVisible(true);
        on_cbResPlotCol_currentIndexChanged(0);
}

void O18MainWnd::on_cbResPlotCol_currentIndexChanged(int index)
{
        if( m_model.state() != ST_FINISH )
                return;
        int theta_index = ui->cbResTheta->currentIndex();
        if( ui->cbResTheta->count() > 1 && theta_index == 0 )
                return;
        --theta_index;
        if(theta_index < 0 || theta_index >= m_model.result().thetas.size() )
        {
                m_resPlot.setVisible(false);
                return;
        }
        const COneThetaResult& otr = m_model.result().result[theta_index];
        --index;
        if( index < 0 || index >= otr.colresults.size() )
        {
                ui->cbResO17->setEnabled(false);
                ui->cbResO18->setEnabled(false);
                m_resPlot.setVisible(false);
                return;
        }
        ui->cbResO17->setEnabled(true);
        ui->cbResO18->setEnabled(true);
        if( !m_resPlot.isVisible() || m_resPlot.plottableCount() != 2 )
        {
                QRect rc = ui->wgtImage->geometry();
                rc.adjust(30, 50, -30, -5);
                m_resPlot.setGeometry(rc);
                m_resPlot.setStyleSheet("border-style: solid;border-width: 2px;\
border-radius: 6px;border-color: rgba(200,200,250, 100);");
                m_resPlot.setBackground(QColor(0, 0, 0, 255));
                for(int i = m_resPlot.plottableCount(); --i >= 0; )
                        m_resPlot.removePlottable(i);
                //revertir ejes
                QCPBars *o18 = new QCPBars(m_resPlot.yAxis, m_resPlot.xAxis);
                QCPBars *o17 = new QCPBars(m_resPlot.yAxis, m_resPlot.xAxis);
                m_resPlot.addPlottable(o18);
                m_resPlot.addPlottable(o17);
                // set names and colors:
                QPen pen;
                pen.setWidthF(1.2);
                pen.setColor(CLRO18B);
                o18->setPen(pen);
                o18->setBrush(QBrush(QColor(CLRO18.red(), CLRO18.green(), CLRO18.blue(), 50)));
                o18->setName("O18");
                pen.setColor(CLRO17B);
                o17->setPen(pen);
                o17->setBrush(QBrush(QColor(CLRO17.red(), CLRO17.green(), CLRO17.blue(), 50)));
                o17->setName("O17");
                // stack bars ontop of each other:
                o18->moveAbove(o17);
                // preparar ejes
                QCPAxis& xa = *m_resPlot.xAxis;
                xa.setAutoTicks(true);
                xa.setAutoTickLabels(true);
                xa.setRange(0., 100.);
                xa.setTickLength(0, 4);
                xa.grid()->setVisible(true);
                QPen apen = xa.basePen();
                apen.setColor(QColor(255, 255, 255));
                xa.setBasePen(apen);
                xa.setTickPen(apen);
                xa.setSubTickPen(apen);
                xa.setAutoSubTicks(false);
                xa.setLabel("");
                QCPAxis& ya = *m_resPlot.yAxis;
                // preparar y eje:
                ya.setPadding(5); // a bit more space to the left border
                ya.grid()->setVisible(false);
                ya.setBasePen(apen);
                ya.setTickPen(apen);
                ya.setSubTickPen(apen);
                ya.setAutoSubTicks(false);
                ya.setTickLabelColor(QColor(255, 255, 255));
                ya.setLabel("");
                xa.setAutoTickStep(true);
                xa.setTickStep(5);
                xa.setTickLabelColor(QColor(255, 255, 255));
                xa.setTickLabels(true); //make it true at step with big charts
                m_resPlot.legend->setVisible(true);
                m_resPlot.setInteractions(0);
                m_resPlot.setVisible(true);
                if(m_resPlotTitle == 0 )
                {
                        m_resPlotTitle = new QCPPlotTitle(&m_resPlot, "");
                        m_resPlot.plotLayout()->insertRow(m_resPlot.plotLayout()->rowCount());
                        m_resPlot.plotLayout()->addElement(m_resPlot.plotLayout()->rowCount() - 1, 0, m_resPlotTitle);
                        m_resPlotTitle->setTextColor(CLRPLOTTEXT);
                }
        }
        onCBResO1718_toggled(true);
}

void O18MainWnd::on_cbResPlotLevel_stateChanged(int)
{
        onCBResO1718_toggled(true);
}

void O18MainWnd::on_tabWidget_currentChanged(int index)
{
        if( m_model.state() == ST_BEGIN || m_model.state() == ST_FINISH )
                return;
        if( index == 0 )
                m_resPlot.setVisible(false);
        else
                drawVolApp();
}

void O18MainWnd::onCBResO1718_toggled(bool)
{
        if( m_model.state() != ST_FINISH )
                return;
        int theta_index = ui->cbResTheta->currentIndex();
        if( ui->cbResTheta->count() > 1 && theta_index == 0 )
                return;
        --theta_index;
        if(theta_index < 0 || theta_index >= m_model.result().thetas.size() )
                return;
        const COneThetaResult& otr = m_model.result().result[theta_index];
        int index = ui->cbResPlotCol->currentIndex() - 1;
        if( index < 0 || index >= otr.colresults.size() )
                return;
        if( !ui->cbResO17->isChecked() && !ui->cbResO18->isChecked() )
        {
                if( sender() == ui->cbResO17 )
                        ui->cbResO18->setChecked(true);
                else
                        ui->cbResO17->setChecked(true);
                return;
        }
        const COneColumnResult& colres = otr.colresults[index];
        QVector<double> yAxis;
        for(int i = 0; i != colres.levels_count; ++i)
                yAxis << i;
        QCPAxis& ya = *m_resPlot.yAxis;
        // preparar y eje:
        ya.setRange(0, colres.levels_count); //default
        ya.setRangeReversed(true);
        ya.setTickLabels(true);
        QCPAxis& xa = *m_resPlot.xAxis;
        xa.setAutoTickStep(true);
        xa.setAutoTickCount(4);
        double xmax = 0;
        double xmin = colres.o18last * 100;
        // Add data:
        QCPBars* o18 = qobject_cast<QCPBars*>(m_resPlot.plottable(0));
        QCPBars* o17 = qobject_cast<QCPBars*>(m_resPlot.plottable(1));
        o18->clearData();
        if( o18 != 0 && ui->cbResO18->isChecked() )
        {
                o18->setVisible(true);
                if( o17 != 0 && ui->cbResO17->isChecked() )
                {
                        QVector<double> pdata = colres.o18concentrations;
                        QVector<double>::iterator itr = pdata.begin(), eitr = pdata.end();
                        QVector<double>::const_iterator o17itr = colres.o17concentrations.begin();
                        for( ;itr != eitr; ++itr, ++o17itr)
                                *itr -= *o17itr;
                        o18->setData(yAxis, pdata);
                }
                else
                {
                        o18->setData(yAxis, colres.o18concentrations);
                }
                find_min_max(xmin, xmax, colres.o18concentrations);
        }
        else if( o18 != 0 )
                o18->setVisible(false);
        o17->clearData();
        if( o17 != 0 && ui->cbResO17->isChecked() )
        {
                o17->setVisible(true);
                o17->setData(yAxis, colres.o17concentrations);
                find_min_max(xmin, xmax, colres.o17concentrations);
        }
        else if( o17 != 0 )
                o17->setVisible(false);
        xa.setRange(ui->cbResPlotLevel->isChecked() ? xmin : 0, xmax);
        if( m_resPlotTitle != 0)
        {
                if( o18 != 0 && ui->cbResO18->isChecked() && o17 != 0 && ui->cbResO17->isChecked() )
                        m_resPlotTitle->setText(tr("Column %1 - O18 & O17").arg(index + 1));
                else if( o17 != 0 && ui->cbResO17->isChecked() )
                        m_resPlotTitle->setText(tr("Column %1 - O17").arg(index + 1));
                else if( o18 != 0 && ui->cbResO18->isChecked() )
                        m_resPlotTitle->setText(tr("Column %1 - O18").arg(index + 1));
        }
        m_resPlot.replot();
}

void O18MainWnd::setupBarChart(QCustomPlot& plot, int column_index)
{
        makeOpacity(plot, 0.5 - column_index * 0.035);
        plot.setBackground(QColor(96, 96, 96, 0));
        for(int i = plot.plottableCount(); --i >= 0; )
                plot.removePlottable(i);
        //revertir ejes
        QCPBars *o18 = new QCPBars(plot.yAxis, plot.xAxis);
        QCPBars *o17 = new QCPBars(plot.yAxis, plot.xAxis);
        plot.addPlottable(o18);
        plot.addPlottable(o17);
        // set names and colors:
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(CLRO18B);
        o18->setPen(pen);
        o18->setBrush(QBrush(QColor(CLRO18.red(), CLRO18.green(), CLRO18.blue(), 50)));
        o18->setName("O18");
        pen.setColor(CLRO17B);
        o17->setPen(pen);
        o17->setBrush(QBrush(QColor(CLRO17.red(), CLRO17.green(), CLRO17.blue(), 50)));
        o17->setName("O17");
        // stack bars ontop of each other:
        o18->moveAbove(o17);
        // preparar ejes
        QCPAxis& xa = *plot.xAxis;
        xa.setAutoTicks(true);
        xa.setAutoTickLabels(true);
        xa.setRange(0., 100.);
        xa.setTickLength(0, 4);
        xa.grid()->setVisible(true);
        QPen apen = xa.basePen();
        apen.setColor(QColor(255, 255, 255));
        xa.setBasePen(apen);
        xa.setTickPen(apen);
        xa.setSubTickPen(apen);
        xa.setAutoSubTicks(false);
        xa.setLabel("");
        QCPAxis& ya = *plot.yAxis;
        // preparar y eje:
        ya.setRange(0, DEFLAYERSCOUNT); //default
        ya.setPadding(5); // a bit more space to the left border
        ya.grid()->setVisible(false);
        ya.setBasePen(apen);
        ya.setTickPen(apen);
        ya.setSubTickPen(apen);
        ya.setAutoSubTicks(false);
        ya.setTickLabelColor(QColor(255, 255, 255));
        ya.setTickLabels(false);
        ya.setLabel("");
        xa.setAutoTickStep(false);
        xa.setTickStep(25);
        xa.setTickLabelColor(QColor(255, 255, 255));
        xa.setTickLabels(true); //make it true at step with big charts
        // Add data:
        QVector<double> yAxis, o18Data, o17Data;
        ChartCtrl& cc = m_chartsgeomery[0][column_index];
        int LayersCount = DEFLAYERSCOUNT;
        int plotHeight = cc.geometry.height();
        if( column_index != 0 )
                LayersCount = ((double)plotHeight) / plotHeight * DEFLAYERSCOUNT + .5;
        for(int i = 0; i != LayersCount; ++i)
        {
                yAxis << i;
                double dt = random() * 100.;
                o18Data << dt - dt/2;
                o17Data << dt / 2.;
        }
        o18->setData(yAxis, o18Data);
        o17->setData(yAxis, o17Data);
        // setup legend:
        plot.legend->setVisible(false);
        plot.setToolTip(tr("Column #%1").arg(column_index + 1));
        plot.setInteractions(0);
}

void O18MainWnd::saveChartGeometry(int, int)
{
        QSettings sets(m_iniFileName, QSettings::IniFormat);
        sets.setValue("step_count", m_chartsgeomery.size());
        for(int i = 0; i != m_chartsgeomery.size(); ++i)
        {
                const ChartCtrls& ccs = m_chartsgeomery[i];
                for(int j = 0; j != COLCOUNT; ++j)
                {
                        QString group = QString("step%1_col%2").arg(i).arg(j);
                        sets.beginGroup(group);
                        const ChartCtrl& cc = ccs[j];
                        sets.setValue("isvisible", cc.isVisible);
                        const QRect& geom = cc.geometry;
                        sets.setValue("left", geom.left());
                        sets.setValue("top", geom.top());
                        sets.setValue("widht", geom.width());
                        sets.setValue("height", geom.height());
                        sets.endGroup();
                        group = QString("step%1_col%2_finished").arg(i).arg(j);
                        sets.beginGroup(group);
                        sets.setValue("isvisible", cc.isVisible_finished);
                        const QRect& geomf = cc.geometry_finished;
                        sets.setValue("left", geomf.left());
                        sets.setValue("top", geomf.top());
                        sets.setValue("widht", geomf.width());
                        sets.setValue("height", geomf.height());
                        sets.endGroup();
                }
        }
}

void O18MainWnd::loadChartGeometry(int, int)
{
        QSettings sets(m_iniFileName, QSettings::IniFormat);
        int stepcnt = sets.value("step_count", 1).toInt();
        if( stepcnt <= 0 || stepcnt > 100 )
                stepcnt = 1;
        m_chartsgeomery.resize(stepcnt);
        for(int i = 0; i != m_chartsgeomery.size(); ++i)
        {
                ChartCtrls& ccs = m_chartsgeomery[i];
                ccs.resize(COLCOUNT);
                for(int j = 0; j != COLCOUNT; ++j)
                {
                        QString group = QString("step%1_col%2").arg(i).arg(j);
                        sets.beginGroup(group);
                        ChartCtrl& cc = ccs[j];
                        cc.isVisible = sets.value("isvisible", true).toBool();
                        int plotHeight = defPlotHeight - j * defPlotDHeight;
                        int h = sets.value("height", plotHeight).toInt();
                        if( h < 10 )
                                h = 10;
                        int w = sets.value("widht", defPlotWidth).toInt();
                        if( w < 10 )
                                w = 10;
                        int l = sets.value("left", defPlotLeft + j * defPlotStep).toInt();
                        int t = sets.value("top", defPlotBase - h).toInt();
                        cc.geometry = QRect(l, t, w, h);
                        sets.endGroup();
                        group = QString("step%1_col%2_finished").arg(i).arg(j);
                        sets.beginGroup(group);
                        cc.isVisible_finished = sets.value("isvisible", cc.isVisible).toBool();
                        h = sets.value("height", h).toInt();
                        if( h < 10 )
                                h = 10;
                        w = sets.value("widht", w).toInt();
                        if( w < 10 )
                                w = 10;
                        l = sets.value("left", l).toInt();
                        t = sets.value("top", t).toInt();
                        cc.geometry_finished = QRect(l, t, w, h);
                        sets.endGroup();
                }
        }
}

void O18MainWnd::ChartGeometry2UI()
{
        ui->sbChartNum->blockSignals(true);
        ui->sbStepNum->blockSignals(true);
        ui->cbVisible->blockSignals(true);
        ui->sbLeft->blockSignals(true);
        ui->sbTop->blockSignals(true);
        ui->sbWidth->blockSignals(true);
        ui->sbHeight->blockSignals(true);
        ui->sbStepNum->setValue(m_curStep);
        const ChartCtrls& ccs = m_chartsgeomery[m_curStep + 1];
        int chart = ui->sbChartNum->value();
        const ChartCtrl& cc = ccs[chart];
        QRect ccg = ui->cbDebugFinished->isChecked() ? cc.geometry_finished : cc.geometry;
        ui->cbVisible->setChecked(ui->cbDebugFinished->isChecked() ? cc.isVisible_finished : cc.isVisible);
        ui->sbLeft->setValue(ccg.left());
        ui->sbTop->setValue(ccg.top());
        ui->sbWidth->setValue(ccg.width());
        ui->sbHeight->setValue(ccg.height());
        ui->sbChartNum->blockSignals(false);
        ui->sbStepNum->blockSignals(false);
        ui->cbVisible->blockSignals(false);
        ui->sbLeft->blockSignals(false);
        ui->sbTop->blockSignals(false);
        ui->sbWidth->blockSignals(false);
        ui->sbHeight->blockSignals(false);
        for(chart = 0; chart != ccs.size(); ++chart)
                adjustPlotPosition(m_curStep, chart, ui->cbDebugFinished->isChecked());
        setColBackground(m_curStep, true, ui->cbDebugFinished->isChecked());
}

void O18MainWnd::UI2ChartGeometry(int chart)
{
        ChartCtrls& ccs = m_chartsgeomery[m_curStep + 1];
        ChartCtrl& cc = ccs[chart];
        if( ui->cbDebugFinished->isChecked() )
                cc.isVisible_finished = ui->cbVisible->isChecked();
        else
                cc.isVisible = ui->cbVisible->isChecked();
        QRect& ccg = ui->cbDebugFinished->isChecked() ? cc.geometry_finished : cc.geometry;
        ccg = QRect(ui->sbLeft->value(), ui->sbTop->value(),
                                                ui->sbWidth->value(), ui->sbHeight->value());
        adjustPlotPosition(m_curStep, chart, ui->cbDebugFinished->isChecked());
}

void O18MainWnd::adjustPlotPosition(int state, int chart, bool finished)
{
        QCustomPlot* plot = m_plots[chart];
        const ChartCtrl& cc = m_chartsgeomery[state + 1][chart];
        plot->setVisible(finished ? cc.isVisible_finished : cc.isVisible);
        plot->setGeometry(finished ? cc.geometry_finished : cc.geometry);
        if( plot->isVisible() )
        {
                QCPAxis& xa = *plot->xAxis;
                xa.rescale(true);
                xa.setAutoTickCount(2);
                plot->replot();
        }
}

void O18MainWnd::setColBackground(int state, bool prepare, bool finished)
{
        ui->lblDemo->setVisible(state == ST_BEGIN);
        static const QString rcnames[] = {
                 "col1-40", "col1-hl-40", "col1-all-40"
                ,"col2-40","col2-hl-40","col2-all-40"
                ,"col3-40","col3-hl-40","col3-all-40"
                ,"col4-50","col4-hl-50","col4-all-50"
                ,"col5-50","col5-hl-50","col5-all-50"
                ,"col6-50","col6-hl-50","col6-all-50"
                ,"col7-60","col7-hl-60","col7-all-60"
                ,"col8-60","col8-hl-60","col8-all-70"
                ,"col9-70","col9-hl-70","col9-all-70"};
        if( state < 0 || state >= (int)(sizeof(rcnames)/sizeof(rcnames[0])/3) )
        {
                ui->lblImage->setPixmap(m_imagenes["begin"]);
                return;
        }
        if( prepare && finished )
                prepare = false;
        int index = state *3 + (finished ? 2 : (prepare ? 0 : 1));
        const QPixmap& px = m_imagenes[rcnames[index]];
        ui->lblImage->setPixmap(px);
}

void O18MainWnd::resizeEvent(QResizeEvent* evt)
{
        QMainWindow::resizeEvent(evt);
        ui->labelTitle_2->setText(ui->labelTitle->text());
        ui->labelTitle->move((ui->wgtImage->width() - ui->labelTitle->width()) / 2, 10);
        ui->labelTitle_2->move(ui->labelTitle->geometry().left() + 2, ui->labelTitle->geometry().top() + 2);
        ui->labelTitle_2->raise();
        ui->labelTitle->raise();
}

void O18MainWnd::makeOpacity(QWidget& wgt, const QBrush& mask)
{
        QGraphicsOpacityEffect& effect = *new QGraphicsOpacityEffect(&wgt);
        effect.setOpacityMask(mask);
        wgt.setGraphicsEffect(&effect);
        wgt.setAutoFillBackground(true);
}

void O18MainWnd::makeOpacity(QWidget& wgt, double opacity)
{
        QGraphicsOpacityEffect& effect = *new QGraphicsOpacityEffect(&wgt);
        effect.setOpacity(opacity);
        wgt.setGraphicsEffect(&effect);
        wgt.setAutoFillBackground(true);
}

void O18MainWnd::closeEvent(QCloseEvent *event)
{
        event->ignore();
        if( m_model.state() != ST_BEGIN && m_model.state() != ST_FINISH )
        {
                if( QMessageBox::warning(this, this->windowTitle(),
                                                         tr("The calculations are going now. Terminate and exit?"),
                                                                 QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes )
                        return;
                m_model.calculation_terminate();
        }
        else if( QMessageBox::warning(this, this->windowTitle(),
                                                                  tr("Exit from the program?"),
                                                                          QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes )
                        return;
        m_bClosing = true;
        event->accept();
}

void O18MainWnd::logMessage(QString msg, int level)
{
        if(level == 0 )
                m_log.append( msg );
        else if( level == 1)
                m_log.append( tr("&nbsp;&nbsp;%1").arg(msg) );
        else if( level == 2)
                m_log.append( tr("&nbsp;&nbsp;&nbsp;&nbsp;%1").arg(msg) );
        else if( level == 3)
                m_log.append( tr("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%1").arg(msg) );
        else
                m_log.append( tr("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%1").arg(msg) );
        ui->tbLog->setHtml( QString("<html>%1</html>").arg(m_log.join( QByteArray("<br>") )) );
        ui->tbLog->moveCursor(QTextCursor::End);
}

void O18MainWnd::logClear()
{
        ui->tbLog->clear();
        m_log.clear();
}

void O18MainWnd::start()
{
        int First_mtd = ui->cbFirstColMTD->currentIndex();
        int Interm_mtd = ui->cbIntermColMTD->currentIndex();
        int Last_mtd = ui->cbLastColMTD->currentIndex();
        if( (First_mtd < 0 || First_mtd >= (int)m_model.mtdevices().size()) ||
                        (Interm_mtd < 0 || Interm_mtd >= (int)m_model.mtdevices().size()) ||
                        (Last_mtd < 0 || Last_mtd >= (int)m_model.mtdevices().size())
                        )
        {
                QMessageBox::critical(this, this->windowTitle(),
                                                          tr("Illegal value of used MTDs"));
                return;
        }
        m_model.set_mtds(First_mtd, Interm_mtd, Last_mtd);
        m_model.set_target_output(ui->sbTargetOutput->value());
        m_model.set_initial_height(ui->sbInitialHeight->value());
        m_model.set_initial_pressure(ui->sbInitialPressure->value());
        m_model.set_o18_initial_concentration(ui->sbInitialO18Conc->value());
        m_model.set_o17_initial_concentration(ui->sbInitialO17Conc->value());
        m_model.set_o18_stop_last_column(ui->sbStopO18Conc->value());
        m_model.set_theta_interm(ui->sbThetaIntermColumn->value());
        m_model.set_theta_1st_start(ui->sbThetaBegin->value());
        m_model.set_theta_1st_end(ui->sbThetaEnd->value());
        m_model.set_theta_1st_step(ui->sbThetaStep->value());
        m_model.set_formula_type(ui->cbFormulaType->currentIndex());
        {
                QSettings sets(m_iniFileName, QSettings::IniFormat);
                m_model.parameters_save(sets);
        }
        m_demoTimer.stop();
        ui->btnStartCalc->setText(tr("Abort"));
        ui->btnStartCalc->setToolTip(tr("Abort calculation"));
        ui->wgtTables->setEnabled(false);
        ui->wgtCtrl->setEnabled(false);
        ui->tabWidget->setCurrentIndex(TAB_SETTINGS);

        logClear();
        logMessage(tr("Start calculation..."));
        ui->tbLog->setVisible(true);
        for(int i = 0; i != m_plots.size(); ++i)
        {
                QCustomPlot& plot = *m_plots[i];
                plot.setVisible(false);

                makeOpacity(plot, 0.5);
                QCPAxis& xa = *plot.xAxis;
                xa.setAutoTickStep(true);
                xa.setTickLabels(true); //make it true at step with big charts
                xa.setAutoTickCount(1);
                QTableWidget& tbl = *m_tables[i];
                clearTable(tbl);
                tbl.setVisible(false);

        }

        ui->cbResTheta->clear();
        m_resPlot.setVisible(false);
        ui->wgtTables->setVisible(false);
        ui->tblVolApp->setVisible(false);
        ui->tblVolApp->setRowCount(0);
        m_model.calculation_start();

}

void O18MainWnd::abort()
{
        logMessage(tr("Aborted by user."));
        m_model.calculation_terminate();
}

void O18MainWnd::clearTable(QTableWidget &tbl)
{
        tbl.setRowCount(1);
        for(int i = 0; i != tbl.columnCount(); ++i)
        {
                QTableWidgetItem* item = tbl.item(0, i);
                if( item != 0 )
                        item->setText(QString());
        }     
}

void O18MainWnd::setTableValue(QTableWidget &tbl, int row, int col, QString value)
{
        if( row >= tbl.rowCount() )
                tbl.setRowCount( row + 1 );
        QTableWidgetItem* item = tbl.item(row, col);
        if( item == 0 )
        {
                item = new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                tbl.setItem(row, col, item);
        }
        item->setText(value);
}

void O18MainWnd::find_min_max(double& min, double& max, const QVector<double>& data)
{
        QVector<double>::const_iterator itr = data.begin(), eitr = data.end();
        for( ; itr != eitr; ++itr)
        {
                if( *itr < min )
                        min = *itr;
                if( *itr > max )
                        max = *itr;
        }
}

void O18MainWnd::drawVolApp()
{
        double ymin = 0, ymax = 0;
        QVector<double> xData;
        QVector<double> yData;
        clearTable(*ui->tblVolApp);
        for( int theta_index = 0; theta_index != m_model.result().result.size(); ++theta_index )
        {
                const COneThetaResult& otr = m_model.result().result[theta_index];
                if( !otr.result_ready )
                        break;
                xData.push_back( m_model.result().thetas[theta_index] );
                double vol = 0.;
                for(int col_index = 0; col_index != otr.colresults.size(); ++col_index)
                        vol += otr.colresults.at(col_index).volapparature;
                if( theta_index == 0  )
                        ymin = ymax = vol;
                else if( ymin > vol )
                        ymin = vol;
                else if( ymax < vol )
                        ymax = vol;
                yData.push_back(vol);
                setTableValue(*ui->tblVolApp, theta_index, 0, tr("%1").arg(xData.last()));
                setTableValue(*ui->tblVolApp, theta_index, 1, tr("%1").arg(yData.last()));
        }
        ui->cbResO17->setEnabled(false);
        ui->cbResO18->setEnabled(false);
        ui->tblVolApp->setVisible(false);
        if( xData.empty() )
                return;
        if( fabs(ymax-ymin) > 0 )
        {
                ymax += fabs(ymax-ymin)/10.;
                ymin -= fabs(ymax-ymin)/10.;
        }
        ui->tblVolApp->repaint();
        ui->tblVolApp->setVisible(true);
        if( !m_resPlot.isVisible() || m_resPlot.plottableCount() != 1 )
        {
                QRect rc = ui->wgtImage->geometry();
                rc.adjust(30, 50, -30, -5);
                m_resPlot.setGeometry(rc);
                m_resPlot.setStyleSheet("border-style: solid;border-width: 2px;\
border-radius: 6px;border-color: rgba(200,200,250, 100);");
                m_resPlot.setBackground(QColor(0, 0, 0, 255));
                for(int i = m_resPlot.plottableCount(); --i >= 0; )
                        m_resPlot.removePlottable(i);
                //revertir ejes
                QCPBars* volApp = new QCPBars(m_resPlot.xAxis, m_resPlot.yAxis);
                m_resPlot.addPlottable(volApp);
                // set names and colors:
                QPen pen;
                pen.setWidthF(1.2);
                pen.setColor(CLRVOLAPPB);
                volApp->setPen(pen);
                volApp->setBrush(QBrush(QColor(CLRVOLAPP.red(), CLRVOLAPP.green(), CLRVOLAPP.blue(), 50)));
                volApp->setName("Volume Apparature");
                // preparar ejes
                QCPAxis& xa = *m_resPlot.xAxis;
                xa.setAutoTicks(true);
                xa.setAutoTickLabels(true);
                xa.setRange(xData.first() - m_model.theta_1st_step(), xData.last()
                                        + m_model.theta_1st_step());
                xa.setTickLength(0, 4);
                xa.grid()->setVisible(true);
                QPen apen = xa.basePen();
                apen.setColor(QColor(255, 255, 255));
                xa.setBasePen(apen);
                xa.setTickPen(apen);
                xa.setSubTickPen(apen);
                xa.setAutoSubTicks(false);
                xa.setLabel(tr("theta"));
                xa.setLabelColor(CLRPLOTTEXT);
                xa.setAutoTickStep(true);
                xa.setAutoTickCount(5);
                QCPAxis& ya = *m_resPlot.yAxis;
                // preparar y eje:
                ya.setPadding(5); // a bit more space to the left border
                ya.grid()->setVisible(false);
                ya.setBasePen(apen);
                ya.setTickPen(apen);
                ya.setSubTickPen(apen);
                ya.setAutoSubTicks(false);
                ya.setTickLabelColor(QColor(255, 255, 255));
                xa.setAutoTickStep(true);
                xa.setTickStep(5);
                xa.setTickLabelColor(QColor(255, 255, 255));
                xa.setTickLabels(true); //make it true at step with big charts
                m_resPlot.legend->setVisible(false);
                m_resPlot.setInteractions(0);
                m_resPlot.setVisible(true);
                if(m_resPlotTitle == 0 )
                {
                        m_resPlotTitle = new QCPPlotTitle(&m_resPlot, "");
                        m_resPlot.plotLayout()->insertRow(m_resPlot.plotLayout()->rowCount());
                        m_resPlot.plotLayout()->addElement(m_resPlot.plotLayout()->rowCount() - 1, 0, m_resPlotTitle);
                        m_resPlotTitle->setTextColor(CLRPLOTTEXT);
                        m_resPlotTitle->setText(tr("Volume of Apparature"));
                }
        }
        QCPAxis& ya = *m_resPlot.yAxis;
        // preparar y eje:
        ya.setRange(ymin, ymax); //default
        ya.setRangeReversed(false);
        ya.setTickLabels(true);
        QCPBars* volApp = qobject_cast<QCPBars*>(m_resPlot.plottable(0));
        volApp->clearData();
        if( volApp != 0 )
        {
                volApp->setWidthType(QCPBars::wtPlotCoords);
                volApp->setWidth(m_model.theta_1st_step() / 10. * 8.);
                volApp->setVisible(true);
                volApp->setData(xData, yData);
        }
        m_resPlot.replot();
}

void O18MainWnd::saveToFile(const QString name)
{
        QFile file(name);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
                m_exportDir = QDir(file.fileName()).absolutePath();
                QSettings sets(m_iniFileName, QSettings::IniFormat);
                sets.setValue("export_dir", m_exportDir);
                sets.sync();
                const CResult& res = m_model.result();
                bool err = file.write(QString(
"O18 calculation results.\n"
"Date: %1\nTime: %2\nTheta: from %3 upto %4, step %5\n").arg(QDate::currentDate().toString())
.arg(QTime::currentTime().toString()).arg(res.theta_1st_start)
.arg( res.theta_1st_start + res.theta_1st_step * res.theta_1st_count)
.arg(res.theta_1st_step).toLocal8Bit()
                                                          ) <= 0;
                if( !err && m_model.terminated() )
                        err = file.write("***=== Aborted by user ===***\n") <= 0;
                const mtdevices_t& mtds = m_model.mtdevices();
                const MTDevice& mtd = mtds[ m_model.first_mtd()];
                if( !err )
                {
                        static const QString cols[] = { tr("first"), tr("interm."), tr("last") };
                        static const QString mtdout = "MTD for %1 column: ;%2; throughput ;%3; kg/m^2/hour; height ;%4; m; delta pressure ;%5; kPa\n";
                        err = file.write(mtdout.arg(cols[0]).arg(mtd.name).arg(mtd.throughput)
                                                                         .arg(mtd.height).arg(mtd.dpressure).toLocal8Bit()) <= 0;
                        const MTDevice& mtd = mtds[ m_model.interm_mtd()];
                        if( !err )
                        {
                                err = file.write(mtdout.arg(cols[1]).arg(mtd.name).arg(mtd.throughput)
                                                                 .arg(mtd.height).arg(mtd.dpressure).toLocal8Bit()) <= 0;
                                const MTDevice& mtd = mtds[ m_model.last_mtd()];
                                if( !err )
                                {
                                        err = file.write(mtdout.arg(cols[2]).arg(mtd.name).arg(mtd.throughput)
                                                                         .arg(mtd.height).arg(mtd.dpressure).toLocal8Bit()) <= 0;
                                }
                        }
                }
                for(int th = 0; !err && th != res.result.size(); ++th)
                {
                        const COneThetaResult& th1res = res.result.at(th);
                        const QVector<COneColumnResult>& colsres = th1res.colresults;
                        err = file.write(QString("\n Theta 1st: %1; Columns count is:;%2\n")
                                                         .arg(res.thetas[th]).arg(colsres.size()).toLocal8Bit()) <= 0;
                        int max_levels = 0;
                        //headers
                        QString line[5];
                        for(int c = 0; !err && c != colsres.size(); ++c)
                        {
                                if( max_levels < colsres.at(c).levels_count )
                                        max_levels = colsres.at(c).levels_count;
                                line[0] += QString(";Column #%1;;;").arg(c + 1);
                                line[1] += QString(";theta:; #%1;;;").arg(colsres.at(c).theta);
                                line[2] += QString(";MTD:; %1;;;").arg(colsres.at(c).mtd_name);
                                line[3] += QString(";levels: %1;;;").arg(colsres.at(c).levels_count);
                                line[4] += ";P;T;A18;O18;O17;";
                        }
                        for( int i = 0; !err && i != 5; ++i)
                                if( !line[i].isEmpty() )
                                {
                                        line[i] = ";" + line[i] +"\n";
                                        err = file.write(line[i].toLocal8Bit()) <= 0;
                                }
                        for(int l = 0; !err && l != max_levels; ++l)
                        {
                                QString line = QString("%1").arg(l);
                                for(int c = 0; c != colsres.size(); ++c)
                                {
                                        const COneColumnResult& ocr = colsres.at(c);
                                        if( l >=  ocr.levels_count )
                                        {
                                                line += ";;;;;;";
                                                continue;
                                        }
                                        line += QString(";;%1;%2;%3;%4;%5").arg(ocr.pressures[l])
                                                        .arg(ocr.temperatures[l]).arg(ocr.alphas[l])
                                                        .arg(ocr.o18concentrations[l]).arg(ocr.o17concentrations[l]);
                                }
                                if( !line.isEmpty() )
                                {
                                        line += "\n";
                                        err = file.write(line.toLocal8Bit()) <= 0;
                                }
                        }
                }
                file.write(QString("\n\nVolume of apparature\nTheta;VA;").toLocal8Bit());
                for(int th = 0; !err && th != res.result.size(); ++th)
                {
                        const COneThetaResult& th1res = res.result.at(th);
                        if( !th1res.result_ready )
                                break;
                        double vol = 0.;
                        for(int col_index = 0; col_index != th1res.colresults.size(); ++col_index)
                                vol += th1res.colresults.at(col_index).volapparature;
                        err = file.write( QString("\n%1;%2").arg(res.thetas[th])
                                                .arg(vol).toLocal8Bit() ) <= 0;
                }
                file.close();
                if( err )
                        QMessageBox::critical(this, tr("Save results"), tr("Can't write to file\n\"%1\"").arg(name));
                else
                        QMessageBox::information(this, tr("Save results"),
                                                                         tr("The file was written successfully."));
        }
        else
                QMessageBox::critical(this, tr("Save results"), tr("Can't create file \"%1\"").arg(name));
}

void O18MainWnd::model_on_prepare_to_step(int cur_theta_index, int state)
{
        setColBackground(state, true, false);
        if( state == ST_CALC_0 )
        {
                logMessage(tr("&theta; %1 ...").arg(m_model.result().thetas[cur_theta_index],
                                                                                                                   0, 'f', 2));
        }
        logMessage(tr("Column %1 ...").arg(state + 1), 1);
        if( state == ST_CALC_0 )
        {
                QString thetaval = tr("%1").arg(m_model.result().thetas.at(cur_theta_index));
                ui->cbResTheta->addItem(thetaval);
                ui->cbResTheta->setCurrentIndex(ui->cbResTheta->count() - 1);
                ui->wgtTables->setVisible(false);
        }
        if( (state + 1) < m_chartsgeomery.size() )
        {
                //state value también es el numero de la columna actual.
                QCustomPlot& plot = *m_plots[state];
                const COneThetaResult& otr = m_model.result().result[cur_theta_index];
                const COneColumnResult& colres = otr.colresults[state];
                QVector<double> yAxis;
                for(int i = 0; i != colres.levels_count; ++i)
                        yAxis << i;
                for(int p = 0; p != plot.plottableCount(); ++p )
                {
                        QCPBars* o = qobject_cast<QCPBars*>(plot.plottable(p));
                        if( o != 0 )
                        {
                                o->clearData();
                                o->setData(yAxis, p == 0 ? colres.o18concentrations : colres.o17concentrations);
                        }
                }
                for(int i = 0; i != m_plots.size(); ++i)
                        adjustPlotPosition(state, i, false);
                QCPAxis& ya = *plot.yAxis;
                ya.setAutoTickStep(true);
                ya.setRange(0, colres.levels_count); //default
                ya.setRangeReversed(true);
                QCPAxis& xa = *plot.xAxis;
                xa.setAutoTickStep(true);
                xa.setAutoTickCount(4);
                xa.setRange(0., colres.o18last);
                plot.replot();
                QTableWidget& tbl = *m_tables[state];
                clearTable(tbl);
                tbl.setVisible(false);
                tbl.setRowCount(colres.levels_count);
                for(int i = 0; i != tbl.rowCount(); ++i)
                        setTableValue(tbl, i, COL_Z, tr("%1").arg(i+1));
                tbl.setVisible(true);
        }
}

void O18MainWnd::model_on_substep_processed(int cur_theta_index, int state, int substep)
{
        setColBackground(state, false, false);
        QCustomPlot& plot = *m_plots[state];
        const COneThetaResult& otr = m_model.result().result[cur_theta_index];
        const COneColumnResult& colres = otr.colresults[state];
        static QVector<double> yAxis;
        if( yAxis.size() != colres.levels_count )
        {
                yAxis.clear();
                for(int i = 0; i != colres.levels_count; ++i)
                        yAxis << i;
        }
        for(int p = 0; p != plot.plottableCount(); ++p )
        {
                QCPBars* o = qobject_cast<QCPBars*>(plot.plottable(p));
                if( o != 0 )
                {
                        o->clearData();
                        if( p == 0 )
                        {
                                QVector<double> pdata = colres.o18concentrations;
                                QVector<double>::iterator itr = pdata.begin(), eitr = pdata.end();
                                QVector<double>::const_iterator o17itr = colres.o17concentrations.begin();
                                for( ;itr != eitr; ++itr, ++o17itr)
                                        *itr -= *o17itr;
                                o->setData(yAxis, pdata);
                        }
                        else
                                o->setData(yAxis, colres.o17concentrations);
                }
        }
        if( !ui->cbBoost->isChecked() || m_model.delay() > 0 || colres.levels_count < 200 || (substep%(colres.levels_count/10)) == 0
                        || substep == (colres.levels_count - 1) )
                plot.replot();
}

void O18MainWnd::model_on_step_processed(int , int state)
{
        if( m_bClosing )
                return;
        setColBackground(state, false, true);
        QCustomPlot& plot = *m_plots[state];
        plot.replot();
}

void O18MainWnd::model_theta_finished(int )
{
        if( m_bClosing )
                return;
        if( m_resPlot.isVisible() || ui->tabWidget->currentIndex() == 1 )
        {
                m_resPlot.setVisible(false);
                m_resPlot.replot();
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                drawVolApp();
        }
}

void O18MainWnd::model_on_finished()
{
        if( m_bClosing )
                return;
        ui->wgtTables->setVisible(false);
        ui->tblVolApp->setVisible(false);
        logMessage("Calculation finished");
        m_resPlot.setVisible(false);
        QTime tm;

        QMessageBox msg(QMessageBox::Information, tr("Sync data"),
                                        tr("Calculation finished, syncing data.\nPlease, wait a moment..."),
                                        QMessageBox::NoButton, ui->wgtImage);
        msg.raise();
        msg.setIconPixmap(QPixmap(BASE_DIR + "/app/update.jpg"));
        msg.setWindowFlags( Qt::FramelessWindowHint );
        makeOpacity(msg, 0.8);
        msg.setStyleSheet("QWidget{background-color: rgb(0, 0, 0); }"
"QMessageBox{border-style: solid;border-width: 2px;border-radius: 6px;"
"border-color: rgba(200,200,250, 100);}");
        if( !m_model.result().result.empty() &&
                        !m_model.result().result.at(0).colresults.empty())
        {
                msg.show();
                msg.repaint();
                /*
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                msg.move( (ui->wgtImage->width() - msg.width())/2,
                                  (ui->wgtImage->height() - msg.height())/2 );
                msg.repaint();
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                */
                tm.start();
        }
        ui->btnStartCalc->setText(tr("Start"));
        ui->btnStartCalc->setToolTip(tr("Start calculation"));
        if( !m_model.result().result.empty() &&
                        !m_model.result().result.at(0).colresults.empty())
        {
                ui->tabWidget->setCurrentIndex(TAB_RESULTS);
                ui->wgtTables->setVisible(true);
                ui->wgtTables->setEnabled(true);
                ui->wgtCtrl->setEnabled(true);
                if( ui->cbResTheta->count() > 1 )
                        ui->cbResTheta->insertItem(0, tr("Vol.App."));
        }
        ui->wgtTables->setEnabled(true);
        ui->wgtCtrl->setEnabled(true);
        ui->tabWidget->update();
        if(ui->cbResTheta->currentIndex() == 0 )
                on_cbResTheta_currentIndexChanged(0);
        else
                ui->cbResTheta->setCurrentIndex(0);
        while( !m_model.result().result.empty() &&
                   !m_model.result().result.at(0).colresults.empty() &&
                   tm.elapsed() < 3000)
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if( !m_model.result().result.empty() &&
                        !m_model.result().result.at(0).colresults.empty() && ui->cbAutoSave->isChecked() )
        {
                if( m_model.terminated() )
                {
                        if( QMessageBox::question(this, tr("Save results"),
                                                                          tr("Calculation finished. Save result to file?"),
                                                                          QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
                        {
                                on_btnExportResult_clicked();
                        }
                        return;
                }
                if( !QDir(m_exportDir).exists() )
                {
                        m_exportDir = QApplication::applicationDirPath();
                        saveToFile( QString("%1/%2.csv").arg(m_exportDir).arg(
                                                        QDateTime::currentDateTime().toString("O18_yyyy-MM-dd_hh-mm-ss")) );
                }
        }
}

void O18MainWnd::model_on_fatal_error(QString err)
{
        if( m_bClosing )
                return;
        logMessage(tr("Calculation terminated due error: %1").arg(err));
        ui->btnStartCalc->setText(tr("Start"));
        ui->btnStartCalc->setToolTip(tr("Start calculation"));
        ui->tabRes->setEnabled(true);
        ui->tabWidget->setCurrentIndex(TAB_RESULTS);
        ui->wgtTables->setVisible(true);
        ui->tblVolApp->setVisible(false);
}
