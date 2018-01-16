#ifndef O18MAINWND_H
#define O18MAINWND_H
#include <QMainWindow>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QTableWidget>
#include "qcustomplot.h"
#include "commondefs.h"
#include "o18model.h"
namespace Ui {
class O18MainWnd;
}
enum {
        TAB_SETTINGS = 0,
        TAB_RESULTS = 1
};
enum {
        COL_Z = 0,
        COL_ALPHA = 1,
        COL_O18 = 2,
        COL_O17 = 3,
};
struct ChartCtrl
{
        bool isVisible;
        QRect geometry;
        bool isVisible_finished;
        QRect geometry_finished;
};
typedef QVector<ChartCtrl> ChartCtrls;
typedef QVector<ChartCtrls> StepChartCtrls;
class O18MainWnd : public QMainWindow
{
        Q_OBJECT
public:
        explicit O18MainWnd(QWidget *parent = 0);
        ~O18MainWnd();
private slots:
        void onDebugPosChanged(int);
        void onDebugVisibleChanged(bool);
        void onDebugStepChanged(int);
        void onDebugChartChanged(int);
        void onDebugPosSave();
        void onDebugFinishedChanged(bool);
        void onBtnAddStepClicked();
        void onDemoTimer();
        void on_btnMTDSetup_clicked();
        void on_btnStartCalc_clicked();
        void on_btnDebugFinCopy_clicked();
        void on_btnCopyPrev_clicked();
        void on_btnExportResult_clicked();
        void on_cbResTheta_currentIndexChanged(int index);
        void on_cbResPlotCol_currentIndexChanged(int index);
        void on_cbResPlotLevel_stateChanged(int);
        void onCBResO1718_toggled(bool);
        void on_tabWidget_currentChanged(int index);
        void model_on_prepare_to_step(int cur_theta_index, int state);
        void model_on_substep_processed(int cur_theta_index, int state, int substep);
        void model_on_step_processed(int cur_theta_index, int state);
        void model_theta_finished(int cur_theta_index);
        void model_on_finished();
        void model_on_fatal_error(QString err);
private:
        void setupBarChart(QCustomPlot& plot, int column_index);
        static inline double random() { return ((double)qrand()) / RAND_MAX; }
        void saveChartGeometry(int step = -1, int chart = -1);
        void loadChartGeometry(int step = -1, int chart = -1);
        void ChartGeometry2UI();
        void UI2ChartGeometry(int chart);
        void adjustPlotPosition(int state, int chart, bool finished);
        void setColBackground(int state, bool prepare, bool finished);
        virtual void resizeEvent(QResizeEvent* evt);
        void makeOpacity(QWidget& wgt, const QBrush& mask);
        void makeOpacity(QWidget* wgt, const QBrush& mask) { makeOpacity(*wgt, mask); }
        void makeOpacity(QWidget& wgt, double opacity);
        void makeOpacity(QWidget* wgt, double opacity) { makeOpacity(*wgt, opacity); }
        void closeEvent(QCloseEvent* event);
        void logMessage(QString msg, int level = 0);
        void logClear();
        void start();
        void abort();
        void clearTable(QTableWidget& tbl);
        void setTableValue(QTableWidget& tbl, int row, int col, QString value);
        static void find_min_max(double& min, double& max, const QVector<double>& data);
        void drawVolApp();
        void saveToFile(const QString fname);
private:
        Ui::O18MainWnd *ui;
        bool m_showPositioning;
        QString m_iniFileName;
        QList<QCustomPlot*> m_plots;
        QList<QTableWidget*> m_tables;
        QMap<QString, QPixmap> m_imagenes;
        StepChartCtrls m_chartsgeomery;
        int m_curStep;
        QTimer m_demoTimer;
        o18model m_model;
        QStringList m_log;
        bool m_bClosing;
        QCustomPlot& m_resPlot;
        QCPPlotTitle* m_resPlotTitle;
        QString m_exportDir;
};
#endif // O18MAINWND_H
