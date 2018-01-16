#ifndef O18MODEL_H
#define O18MODEL_H
#include <QObject>
#include <QSettings>
#include "commondefs.h"
enum {
        ST_FINISH = -2,
        ST_BEGIN = -1,
        ST_CALC_0 = 0,
        ST_CALC_1 = ST_CALC_0 + 1,
        ST_CALC_2 = ST_CALC_1 + 1,
        ST_CALC_3 = ST_CALC_2 + 1,
        ST_CALC_4 = ST_CALC_3 + 1,
        ST_CALC_5 = ST_CALC_4 + 1,
        ST_CALC_6 = ST_CALC_5 + 1,
        ST_CALC_7 = ST_CALC_6 + 1,
        ST_CALC_8 = ST_CALC_7 + 1,
};
static const int ST_CALC_MAX = ST_CALC_8;
typedef std::vector<MTDevice> mtdevices_t;
enum //FORMULA_TYPE
{
        FT_OLD,
        FT_NEW,
        FT_MIX
};
class o18model : public QObject
{
        Q_OBJECT
public:
        explicit o18model(QObject *parent = 0);
        int state() const { return m_state; }
        void calculation_start();
        void calculation_terminate();
        int lastcolindex() const { return m_lastcolindex; }
        int cur_theta_1st_index() const { return m_cur_theta_index; }
        int theta_1st_count() const { return m_theta_count; }
        double theta_1st_current() const { return m_theta_1st_start + m_cur_theta_index * m_theta_1st_step; }
        const mtdevices_t& mtdevices() const { return m_mtdevices; }
        void mtdevices_update(const mtdevices_t& mtds) { m_mtdevices = mtds; }
        void set_mtds(int first_mtd, int interm_mtd, int last_mtd) {
                m_1st_mtd = first_mtd; m_interm_mtd = interm_mtd; m_last_mtd = last_mtd;
        }
        int first_mtd() const { return m_1st_mtd; }
        int interm_mtd() const { return m_interm_mtd; }
        int last_mtd() const { return m_last_mtd; }
        double target_output() const { return m_target_output; }//kg/year
        void set_target_output(double v) { m_target_output = v; }//kg/year
        double initial_height() const { return m_initial_height; }//m
        void set_initial_height(double v) { m_initial_height = v; }//m
        double initial_pressure() const { return m_initial_pressure; }//kPa
        void set_initial_pressure(double v) { m_initial_pressure = v; }//kPa
        double o18_initial_concentration() const { return m_o18_initial_concentration; }//%
        void set_o18_initial_concentration(double v) { m_o18_initial_concentration = v; }//%
        double o17_initial_concentration() const { return m_o17_initial_concentration; }//%
        void set_o17_initial_concentration(double v) { m_o17_initial_concentration = v; }//%
        double o18_stop_last_column() const { return m_o18_stop_last_column; }
        void set_o18_stop_last_column(double v) { m_o18_stop_last_column = v; }
        double theta_interm() const { return m_theta_interm; }
        void set_theta_interm(double v) { m_theta_interm = v; }
        double theta_1st_start() const { return m_theta_1st_start; }
        void set_theta_1st_start(double v) { m_theta_1st_start = v; }
        double theta_1st_end() const { return m_theta_1st_end; }
        void set_theta_1st_end(double v) { m_theta_1st_end = v; }
        double theta_1st_step() const { return m_theta_1st_step; }
        void set_theta_1st_step(double v) { m_theta_1st_step = v; }
        int formula_type() const {return m_formula; } //FORMULA_TYPE
        void set_formula_type(int type) {m_formula = type; } //FORMULA_TYPE
        double mix_conc_thres() const { return m_conc_mix; }//threshold for mix
        void set_mix_conc_thres(double value) { m_conc_mix = value; }//threshold for mix
        void parameters_load(QSettings& sets);
        void parameters_save(QSettings& sets, bool mtdsonly = false) const;
        QStringList mtdevices_names() const;
        const CResult& result() const { return m_result; }
        bool terminated() const { return m_bTerminated; }
        int delay() const { return m_delay; }
signals:
        void prepare_to_step(int cur_theta_index, int state);
        void substep_processed(int cur_theta_index, int state, int substep);
        void step_processed(int cur_theta_index, int state);
        void theta_finished(int cur_theta_index);
        void finished();
        void fatal_error(QString);
public slots:
private:
        void mtdevices_save(QSettings& sets) const;
        void mtdevices_load(QSettings& sets);
        MTDevice* mtd_by_name(const QString& mtdname);
        static double t_from_p(double p);
        static double calc_Xo17(int z, double x0_o17, double x_o18, double eps_o18, double theta, int formula_type, double conc_mix);
        //calculation
        inline bool test_lastcolumn();
        inline bool calculate_substep(int substep); //calc one MTD in the column
        static inline double calc_conc(int z, double x0, double eps, double theta, int formula_type, double conc_mix);
        //hace relativo por exepath si es posible
        static QString make_relative_path(QString path);
private:
        int m_cur_theta_index;
        int m_theta_count;
        int m_state;
        bool m_bTerminated;
        int m_lastcolindex;
        mtdevices_t m_mtdevices;
        int m_1st_mtd, m_interm_mtd, m_last_mtd;
        double m_target_output; //kg/year
        double m_initial_height; //m
        double m_initial_pressure;//kPa
        double m_o18_initial_concentration; //1/mol
        double m_o17_initial_concentration; //1/mol
        double m_theta_interm;
        double m_theta_1st_start;
        double m_theta_1st_end;
        double m_theta_1st_step;
        double m_o18_stop_last_column;//1/mol
        int m_delay; //delay por step, ms or < 0 if no delay
        CResult m_result;
        //helpers
        const MTDevice* m_hcurmtd;
        COneColumnResult* m_hcurcolres;
        double m_alpha_coeff2, m_alpha_coeff1, m_alpha_coeff0;
        int m_formula; //FORMULA_TYPE
        double m_conc_mix; //threshold for mix
};
#endif // O18MODEL_H
