#include "o18model.h"
#include <QApplication>
#include <QStringList>
#include <QThread>
#include <cmath>
#include <algorithm>
o18model::o18model(QObject *parent)
        : QObject(parent)
        , m_state(ST_BEGIN)
        , m_bTerminated(false)
        , m_lastcolindex(-1)
        , m_formula(FT_NEW) //FORMULA_TYPE
        , m_conc_mix(40.) //threshold for mix - 40% default
{
}
void o18model::calculation_start()
{
        m_bTerminated = false;
        if( m_theta_1st_end < m_theta_1st_start )
        {
                double tmp = m_theta_1st_end;
                m_theta_1st_end = m_theta_1st_start;
                m_theta_1st_start = tmp;
        }
        if( m_theta_1st_step < 0.000001)
                m_theta_1st_step = 0.5;
        m_cur_theta_index = 0;
        m_theta_count = (m_theta_1st_end - m_theta_1st_start) / m_theta_1st_step + 1;
        m_result.prepare(m_theta_1st_start, m_theta_1st_step, m_theta_count);
        for( ; m_cur_theta_index != m_theta_count; ++m_cur_theta_index)
        { //theta cicle
                m_lastcolindex = -1;
                m_state = ST_CALC_0;
                COneThetaResult& otr = m_result.result[m_cur_theta_index];
                otr.columnas_count = 0;
                otr.result_ready = false;
                while( true )
                {//column cicle
                        int lastcol = otr.columnas_count;
                        ++otr.columnas_count;
                        otr.colresults.resize(otr.columnas_count);
                        m_hcurcolres = &otr.colresults[lastcol];
                        COneColumnResult& colres = *m_hcurcolres;
                        if( lastcol == 0 )
                        {
                                colres.o18_concentr_in = m_o18_initial_concentration;
                                colres.o17_concentr_in = m_o17_initial_concentration;
                        }
                        else
                        {
                                colres.o18_concentr_in = otr.colresults[lastcol - 1].o18_concentr_out;
                                colres.o17_concentr_in = otr.colresults[lastcol - 1].o17_concentr_out;
                        }
                        colres.pressure_in = m_initial_pressure;
                        colres.height = m_initial_height;
                        double curcoltheta;
                        QString mtdname;
                        if( lastcol == 0 )
                        {
                                mtdname = m_mtdevices[m_1st_mtd].name;
                                curcoltheta = m_result.thetas[m_cur_theta_index];
                        }
                        else
                        {
                                curcoltheta = m_theta_interm;
                                if( colres.o18_concentr_in > m_o18_stop_last_column )
                                        mtdname = m_mtdevices[m_last_mtd].name;
                                else
                                        mtdname = m_mtdevices[m_interm_mtd].name;
                        }
                        otr.thetas.push_back( curcoltheta );
                        colres.theta = curcoltheta;
                        colres.mtd_name = mtdname;
                        m_hcurmtd = mtd_by_name(mtdname);
                        if( m_hcurmtd == 0 )
                        {
                                QString err = tr("The MTD \"%1\" not found for column %2").arg(mtdname).arg(lastcol + 1);
                                emit fatal_error(err);
                                return;
                        }
                        if( m_hcurmtd->height <= 1e-6 )
                        {
                                QString err = tr("The MTD \"%1\" height is not valid value (%2 sm)").arg(mtdname)
                                                .arg(m_hcurmtd->height * 100.);
                                emit fatal_error(err);
                                return;
                        }
                        int levels_count = colres.height / m_hcurmtd->height + 1;
                        if( levels_count == 0 || colres.levels_count >= 100000 )
                        {
                                QString err = tr("The MTD \"%1\" height (%2 sm) and  height of column %3 (%4 m) are not valid values")
                                                .arg(mtdname).arg(m_hcurmtd->height * 100.).arg(lastcol + 1).arg(colres.height);
                                emit fatal_error(err);
                                return;
                        }
                        colres.prepare(levels_count); //resize arrays
                        //calc last x (max., for plot)
                        int substep = levels_count - 1;
                        calculate_substep(substep);
                        m_hcurcolres->pressures[substep] = 0;
                        m_hcurcolres->temperatures[substep] = 0;
                        m_hcurcolres->alphas[substep] = 0;
                        m_hcurcolres->epsilonas[substep] = 0;
                        m_hcurcolres->o18concentrations[substep] = 0;
                        m_hcurcolres->o18_concentr_out = 0;
                        m_hcurcolres->o17concentrations[substep] = 0;
                        m_hcurcolres->o17_concentr_out = 0;
                        emit prepare_to_step(m_cur_theta_index, m_state);
                        QApplication::processEvents();
                        if( m_bTerminated )
                        {
                                otr.clear_last();
                                m_state = ST_FINISH;
                                emit finished();
                                return;
                        }
                        for(substep = 0; substep != levels_count; ++substep)
                        {//MTD or column height cicle
                                if( !calculate_substep(substep) )
                                        return;
                                if( m_delay >= 0 )
                                {
                                        if( m_delay > 100 )
                                        {
                                                int cnt = m_delay / 10;
                                                for(int i = 0; i != cnt; ++i)
                                                {
                                                        QThread::msleep(10);
                                                        QApplication::processEvents();
                                                        if( m_bTerminated )
                                                        {
                                                                m_state = ST_FINISH;
                                                                otr.clear_last();
                                                                emit finished();
                                                                return;
                                                        }
                                                }
                                        }
                                        else
                                        {
                                                QThread::msleep(m_delay);
                                                QApplication::processEvents();
                                                if( m_bTerminated )
                                                {
                                                        m_state = ST_FINISH;
                                                        otr.clear_last();
                                                        emit finished();
                                                        return;
                                                }
                                        }
                                }
                                emit substep_processed(m_cur_theta_index, m_state, substep);
                                QApplication::processEvents();
                                if( m_bTerminated )
                                {
                                        otr.clear_last();
                                        m_state = ST_FINISH;
                                        emit finished();
                                        return;
                                }
                        }
                        if( lastcol == 0 )
                        {
                                colres.flujo = m_target_output / (365 * 24); // kilogramos por hora
                                colres.flujo /= (colres.theta * colres.epsilonas.last() * colres.o18_concentr_in);
                        }
                        else
                        {
                                COneColumnResult& prevcolres = otr.colresults[lastcol - 1];
                                colres.flujo = prevcolres.flujo / prevcolres.o18_concentr_out * prevcolres.o18_concentr_in;
                        }
                        colres.volapparature = colres.height * colres.flujo / m_hcurmtd->throughput;
                        emit step_processed(m_cur_theta_index, m_state);
                        QApplication::processEvents();
                        if( m_bTerminated )
                        {
                                m_state = ST_FINISH;
                                emit finished();
                                return;
                        }
                        if( m_lastcolindex != - 1)
                        {
                                //all calculated normally, finish
                                break;
                        }
                        if( ++m_state > ST_CALC_MAX )
                                break; //overcolumn
                        if( test_lastcolumn() )
                                m_lastcolindex = lastcol + 1; //next column is last
                }
                otr.result_ready = true;
                m_state = ST_FINISH;
                emit theta_finished(m_cur_theta_index);
                QApplication::processEvents();
                if( m_bTerminated )
                        break;
        }
        emit finished();
}
void o18model::calculation_terminate()
{
        m_bTerminated = true;
}
void o18model::parameters_load(QSettings &sets)
{
        mtdevices_load(sets);
        sets.beginGroup("Parameters");
        QStringList mtdnames = mtdevices_names();
        int index = mtdnames.indexOf(sets.value("FirstColMTD", "").toString());
        m_1st_mtd = ( index < 0 || index >= (int)m_mtdevices.size() ) ? 0 : index;
        index = mtdnames.indexOf(sets.value("IntermColMTD", "").toString());
        m_interm_mtd = ( index < 0 || index >= (int)m_mtdevices.size() ) ? 0 : index;
        index = mtdnames.indexOf(sets.value("LastColMTD", "").toString());
        m_last_mtd = ( index < 0 || index >= (int)m_mtdevices.size() ) ? 0 : index;
        m_target_output = sets.value("target_output", 100).toDouble(); //kg/year
        m_initial_height = sets.value("initial_height", 30.).toDouble(); //m
        m_initial_pressure = sets.value("initial_pressure", 100.).toDouble();//kPa
        m_o18_initial_concentration = sets.value("o18_initial_concentration", 0.002).toDouble(); //1/mol
        m_o17_initial_concentration = sets.value("o17_initial_concentration", 0.0001).toDouble(); //1/mol
        m_o18_stop_last_column = sets.value("o18_stop_last_column", 0.1).toDouble(); //1/mol
        m_theta_interm = sets.value("theta_interm", 50.).toDouble(); //%
        m_theta_1st_start = sets.value("theta_1st_start", 35.).toDouble(); //%
        m_theta_1st_end = sets.value("theta_1st_end", 65.).toDouble(); //%
        m_theta_1st_step = sets.value("theta_1st_step", 0.5).toDouble(); //%
        m_delay = sets.value("delay", 10).toInt(); //ms
        sets.endGroup();
        sets.beginGroup("Calc_constants");
        m_alpha_coeff2 = sets.value("alpha_coeff_pow_2", 1991.1).toDouble();
        m_alpha_coeff1 = sets.value("alpha_coeff_pow_1", -4.1887).toDouble();
        m_alpha_coeff0 = sets.value("alpha_coeff_pow_0", 0.0011).toDouble();
        m_formula = sets.value("formula_type", FT_NEW).toInt(); //threshold for mix
        m_conc_mix = sets.value("formula_mix_level", 40).toDouble(); //threshold for mix
        sets.endGroup();
}
void o18model::parameters_save(QSettings& sets, bool mtdsonly) const
{
        mtdevices_save(sets);
        if( mtdsonly )
                return;
        sets.beginGroup("Parameters");
        QString name = (m_1st_mtd >= 0 && m_1st_mtd < (int)m_mtdevices.size()) ?
                                m_mtdevices[m_1st_mtd].name : QString();
        sets.setValue("FirstColMTD",  name);
        name = (m_interm_mtd >= 0 && m_interm_mtd < (int)m_mtdevices.size()) ?
                                m_mtdevices[m_interm_mtd].name : QString();
        sets.setValue("IntermColMTD", name);
        name = (m_last_mtd >= 0 && m_last_mtd < (int)m_mtdevices.size()) ?
                                m_mtdevices[m_last_mtd].name : QString();
        sets.setValue("LastColMTD", name);
        sets.setValue("target_output", m_target_output); //kg/year
        sets.setValue("initial_height", m_initial_height); //m
        sets.setValue("initial_pressure", m_initial_pressure);//kPa
        sets.setValue("o18_initial_concentration", m_o18_initial_concentration); //1/mol
        sets.setValue("o17_initial_concentration", m_o17_initial_concentration); //1/mol
        sets.setValue("o18_stop_last_column", m_o18_stop_last_column); //1/mol
        sets.setValue("theta_interm", m_theta_interm);
        sets.setValue("theta_1st_start", m_theta_1st_start);
        sets.setValue("theta_1st_end", m_theta_1st_end);
        sets.setValue("theta_1st_step", m_theta_1st_step);
        sets.setValue("delay", m_delay); //ms
        sets.endGroup();
        sets.beginGroup("Calc_constants");
        sets.setValue("alpha_coeff_pow_2", m_alpha_coeff2);
        sets.setValue("alpha_coeff_pow_1", m_alpha_coeff1);
        sets.setValue("alpha_coeff_pow_0", m_alpha_coeff0);
        sets.setValue("formula_type", m_formula); //threshold for mix
        sets.setValue("formula_mix_level", m_conc_mix); //threshold for mix
        sets.endGroup();
}
QStringList o18model::mtdevices_names() const
{
        QStringList names;
        mtdevices_t::const_iterator itr = m_mtdevices.begin(), eitr = m_mtdevices.end();
        for( ; itr != eitr; ++itr)
                names << itr->name;
        return names;
}
void o18model::mtdevices_save(QSettings& sets) const
{
        mtdevices_t::const_iterator itr = m_mtdevices.begin(), eitr = m_mtdevices.end();
        for( ; itr != eitr; ++itr)
        {
                QString gr = QString("MTDevice%1").arg( std::distance(m_mtdevices.begin(), itr) );
                sets.beginGroup(gr);
                const MTDevice& mtd = *itr;
                sets.setValue("name", mtd.name);
                sets.setValue("throughput", mtd.throughput);
                sets.setValue("height", mtd.height);
                sets.setValue("dpressure", mtd.dpressure);
                sets.setValue("image", make_relative_path(mtd.image.canonicalFilePath()));
                sets.endGroup();
        }
}
void o18model::mtdevices_load(QSettings& sets)
{
        m_mtdevices.clear();
        for(int i = 0; true; ++i)
        {
                QString gr = QString("MTDevice%1").arg( i );
                sets.beginGroup(gr);
                if( sets.childKeys().isEmpty() )
                {
                        sets.endGroup();
                        return;
                }
                MTDevice mtd;
                mtd.name = sets.value("name", QString("MTD%1").arg(i+1)).toString();
                mtd.throughput = sets.value("throughput", 0.5).toDouble();
                mtd.height = sets.value("height", 0.5).toDouble();
                mtd.dpressure = sets.value("dpressure", 0.5).toDouble();
                QString path = sets.value("image", "").toString();
                if( !path.isEmpty() )
                {
                        QFileInfo fi(path);
                        if( fi.isRelative() )
                        {
                                QFileInfo fa(QApplication::applicationDirPath());
                                QString fas = fa.canonicalPath();
                                if( fas.at( fas.length() - 1 ) != '/' )
                                        fas.append('/');
                                fi = QFileInfo(fas + path);
                        }
                        mtd.image = fi;
                }
                sets.endGroup();
                m_mtdevices.push_back(mtd);
        }
}
struct Predicat : public std::unary_function<MTDevice, bool>
{
        QString seekname;
        Predicat(QString seekname_) : seekname(seekname_) {}
        bool operator ()(const MTDevice& m) const
        {
                return m.name == seekname;
        }
};
MTDevice* o18model::mtd_by_name(const QString& mtdname)
{
        mtdevices_t::iterator itr = std::find_if(m_mtdevices.begin(), m_mtdevices.end(), Predicat(mtdname));
        return itr == m_mtdevices.end() ? (MTDevice*)0 : (&*itr);
}
double o18model::t_from_p(double p)
{
        //pressure kPa
        static double P[] = {5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0,
                                                 55.0, 60.0, 65.0, 70.0, 75.0, 80.0, 85.0, 90.0, 90.5, 91.0,
                                                 91.5, 92.0, 92.5, 93.0, 93.5, 94.0, 94.5, 95.0, 95.5, 96.0,
                                                 96.5, 97.0, 97.5, 98.0, 98.5, 99.0, 99.5, 100.0, 100.5, 101.0,
                                                 101.325, 101.5, 102.0, 102.5, 103.0, 103.5, 104.0, 104.5, 105.0, 105.5,
                                                 106.0, 106.5, 107.0, 107.5, 108.0, 108.5, 109.0, 109.5, 110.0, 115.0,
                                                 121.0, 177.0, 196.0, 223.0};
        static double T[] = {32.88, 45.82, 53.98, 60.07, 64.98, 69.11, 72.70, 75.88, 78.74, 81.34,
                                                 83.73, 85.95, 88.02, 89.96, 91.78, 93.51, 95.15, 96.71, 96.87, 97.02,
                                                 97.17, 97.32, 97.47, 97.62, 97.76, 97.91, 98.06, 98.21, 98.35, 98.50,
                                                 98.64, 98.78, 98.93, 99.07, 99.21, 99.35, 99.49, 99.63, 99.77, 99.91,
                                                 100.00, 100.05, 100.19, 100.32, 100.46, 100.60, 100.73, 100.87, 101.00, 101.14,
                                                 101.27, 101.40, 101.54, 101.67, 101.80, 101.93, 102.06, 102.19, 102.32, 103.59,
                                                 105.0, 116.3, 119.6, 125.0};
        int i = 0;
        for(; i != sizeof(P)/sizeof(P[0]); ++i)
                if( p < P[i] )
                        break;
        if( i == 0 )
                return T[0] + 273.16;
        if( i == sizeof(P)/sizeof(P[0]) )
        {
                //if P too high, calculate by equation:
                return pow(p/1000., 0.2391) * 179.47 + 273.16;//  T[ sizeof(P)/sizeof(P[0]) - 1 ];
        }
        int i0 = i - 1;
        return T[i0] + (T[i] - T[i0])/(P[i] - P[i0]) * (p - P[i0]) + 273.16;
}
double o18model::calc_Xo17(int z, double x0_o17, double x_o18, double eps_o18, double theta, int formula_type, double conc_mix)
{
        static const int ITRCNT = 5;
        double x_o17 = 0;
        for(int i = ITRCNT; --i != 0; )
        {
                double eps_o16 = 0.563 * eps_o18;
                double x_o16 = 1 - (x_o18 + x_o17);
                double eps_o17 = x_o16 * eps_o16 - x_o18 * eps_o18;
                x_o17 = calc_conc(z, x0_o17, eps_o17, theta, formula_type, conc_mix);
        }
        return x_o17;
}
bool o18model::test_lastcolumn()
{
        return m_hcurcolres->o18_concentr_out > m_o18_stop_last_column;
}
bool o18model::calculate_substep(int substep)
{
        double press = m_hcurcolres->pressure_in  + substep * m_hcurmtd->dpressure;
        m_hcurcolres->pressures[substep] = press;
        double temper = t_from_p(press);
        m_hcurcolres->temperatures[substep] = temper;
        double t1 = 1 / temper;
        double ln_alpha = m_alpha_coeff2 * t1 * t1 + m_alpha_coeff1 * t1 + m_alpha_coeff0;
        double alpha = exp(ln_alpha);
        m_hcurcolres->alphas[substep] = alpha;
        if(fabs(alpha) < 1e-8)
        {
                QString err = tr("The invalid value (%1) of alpha was processed for temperature value %2. "
                                         "Coefficients are: 2 %3; 1 %4; 0 %4").arg(alpha).arg(temper)
                                .arg(m_alpha_coeff2).arg(m_alpha_coeff1).arg(m_alpha_coeff0);
                emit fatal_error(err);
                return false;
        }
        double eps_o18 = (alpha - 1) / alpha;
        m_hcurcolres->epsilonas[substep] = eps_o18;
        double theta = m_hcurcolres->theta / 100.;
        //new version
        double x_o18 = calc_conc(substep, m_hcurcolres->o18_concentr_in, eps_o18, theta,
                                                         m_formula, m_conc_mix);
        m_hcurcolres->o18concentrations[substep] = x_o18;
        m_hcurcolres->o18_concentr_out = x_o18;
        //calc o17 concentration;
        double x_o17 = calc_Xo17(substep, m_hcurcolres->o17_concentr_in, x_o18, eps_o18, theta,
                                                         m_formula, m_conc_mix);
        m_hcurcolres->o17concentrations[substep] = x_o17;
        m_hcurcolres->o17_concentr_out = x_o17;
        if( substep == (m_hcurcolres->levels_count - 1) )
        {
                m_hcurcolres->o18last = x_o18;
                m_hcurcolres->o17last = x_o17;
        }
        return true;
}
double o18model::calc_conc(int z, double x0, double eps, double theta, int formula_type, double conc_mix)
{
        switch( formula_type) {
        case FT_NEW:
                {
                        double X_e = (1. - x0 * theta);
                        double X = 1. + exp(z * eps * X_e) * (1. - theta) / X_e;
                        X *= x0 / (1. - x0);
                        return X / (1. + X);
                }
        case FT_OLD:
                return x0 * (1. + (exp(z * eps) - 1.) * (1. - theta) );
        default:
                break;
        }
        //case FT_MIX:
        return calc_conc(z, x0, eps, theta, ( x0 < (conc_mix/100.) ) ? FT_OLD : FT_NEW, conc_mix);
}
QString o18model::make_relative_path(QString path)
{
        QFileInfo fi(path);
        if( fi.isAbsolute() )
        {
                QFileInfo fa(QApplication::applicationDirPath());
                QString fis = fi.canonicalFilePath();
                QString fas = fa.canonicalPath();
                if( fas.at( fas.length() - 1 ) != '/' )
                        fas.append('/');
                if( fis.indexOf(fas) == 0 )
                        return fis.right(fis.length() - fis.length());
        }
        return path;
}
