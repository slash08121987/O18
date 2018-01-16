#ifndef COMMONDEFS_H
#define COMMONDEFS_H
#include <QString>
#include <QVector>
#include <QFileInfo>
static const int COLMAX = 9;
static const int COLCOUNT = COLMAX;
static const int DEFLAYERSCOUNT = 30;
static const QString BASE_DIR = "C:/Users/Burygin.vyacheslav/Documents/Andronov";
struct MTDevice
{
        QString name;
        double throughput; //kg/m^2/hour
        double height;     //m
        double dpressure;    //pressure drop, kPa
        QFileInfo image;
        bool operator ==(const MTDevice& rhs) const
        {
                return name == rhs.name &&
                                throughput == rhs.throughput &&
                                height == rhs.height &&
                                dpressure == rhs.dpressure &&
                                image == rhs.image;
        }
        bool operator !=(const MTDevice& rhs) const
        {
                return !((*this) == rhs);
        }
};
struct COneColumnResult
{
        double theta;
        double pressure_in;
        double flujo;
        double volapparature;
        double o18_concentr_in, o18_concentr_out; //1/mol
        double o17_concentr_in, o17_concentr_out; //1/mol
        double o18last, o17last;
        int levels_count;
        double height; //m
        QString mtd_name;
        QVector<double> o18concentrations, o17concentrations; //1/mol
        QVector<double> temperatures; //Celsius
        QVector<double> pressures; //kPa
        QVector<double> alphas; //u.e.
        QVector<double> epsilonas; //u.e.
        void prepare(int levelscnt)
        {
                levels_count = levelscnt;
                o18concentrations = o17concentrations = temperatures = \
                                pressures = alphas = epsilonas = QVector<double>(levels_count, 0.);
        }
};
struct COneThetaResult
{
        int columnas_count;
        QVector<double> thetas; //thetas por columna
        QVector<COneColumnResult> colresults;
        bool result_ready;
        void clear_last()
        {
                if( columnas_count > 0 )
                {
                        --columnas_count;
                        thetas.resize(columnas_count);
                        colresults.resize(columnas_count);
                }
        }
};
struct CResult
{
        typedef QVector<COneThetaResult> result1theta_t;
        result1theta_t result;
        QVector<double> thetas;
        double theta_1st_start;
        double theta_1st_step;
        int theta_1st_count;
        void clear() {
                result.clear(); thetas.clear();
        }
        void prepare(double theta_1st_start_, double theta_1st_step_, int theta_1st_count_)
        {
                clear();
                theta_1st_start = theta_1st_start_;
                theta_1st_step = theta_1st_step_;
                theta_1st_count = theta_1st_count_;
                for(int i = 0; i != theta_1st_count; ++i)
                {
                        COneThetaResult uno_result;
                        uno_result.columnas_count = 0;
                        uno_result.result_ready = false;
                        //theta the 1st column
                        double theta = theta_1st_start + i * theta_1st_step;
                        thetas.push_back(theta);
                        result.push_back(uno_result);
                }
        }
};
#endif // COMMONDEFS_H
