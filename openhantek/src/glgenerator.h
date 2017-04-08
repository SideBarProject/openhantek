////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the GlScope class.
//
//  Copyright (C) 2008, 2009  Oleg Khudyakov
//  prcoder@potrebitel.ru
//  Copyright (C) 2010  Oliver Haag
//  oliver.haag@gmail.com
//
//  This program is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation, either version 3 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////


#ifndef GLGENERATOR_H
#define GLGENERATOR_H


#include <deque>
#include <memory>
#include <vector>

#include <QObject>

#include "parameters.h"

#define DIVS_TIME                  10.0 ///< Number of horizontal screen divs
#define DIVS_VOLTAGE                8.0 ///< Number of vertical screen divs
#define DIVS_SUB                      5 ///< Number of sub-divisions per div

namespace DSOAnalyser {
    class DataAnalyzer;
}
class OpenHantekSettings;
class GlScope;


////////////////////////////////////////////////////////////////////////////////
///
/// \brief Generates the vertex arrays for the GlScope classes.
class GlGenerator : public QObject {
    Q_OBJECT

    friend class GlScope;

    public:
        GlGenerator(QObject *parent = 0);
        void generateGraphs(OpenHantekSettings *settings, std::shared_ptr<DSOAnalyser::DataAnalyzer>& dataAnalyzer);

    protected:
        void generateGrid();

    private:
        std::vector<std::deque<std::vector<float>>> vaChannel[CHANNELMODE_COUNT];
        std::vector<float> vaGrid[3];
        unsigned int digitalPhosphorDepth = 0;

    signals:
        void graphsGenerated(); ///< The graphs are ready to be drawn
};


#endif
