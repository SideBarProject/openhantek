////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  glgenerator.cpp
//
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


#include <QGLWidget>
#include <QDebug>
#include "glgenerator.h"
#include "settings.h"
#include <math.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////
// class GlArray
/// \brief Initializes the array.
GlArray::GlArray() {
	this->data = 0;
	this->size = 0;
}

/// \brief Deletes the array.
GlArray::~GlArray() {
	if(this->data)
		delete this->data;
}

/// \brief Get the size of the array.
/// \return Number of array elements.
unsigned int GlArray::getSize() {
	return this->size;
}

/// \brief Set the size of the array.
/// Previous array contents are lost.
/// \param size New number of array elements.
void GlArray::setSize(unsigned int size) {
	if(this->size == size)
		return;
	
	if(this->data)
		delete[] this->data;
	if(size)
		this->data = new GLfloat[size];
	else
		this->data = 0;
	
	this->size = size;
}


////////////////////////////////////////////////////////////////////////////////
// class GlGenerator
/// \brief Initializes the scope widget.
/// \param settings The target settings object.
/// \param parent The parent widget.
GlGenerator::GlGenerator(DsoSettings *settings, QObject *parent) : QObject(parent) {
	this->settings = settings;
    qDebug() << "GlGenerator settings: " << this->settings->scope.horizontal.format;
	this->digitalPhosphorDepth = 0;
	
	this->generateGrid();
}

/// \brief Deletes OpenGL objects.
GlGenerator::~GlGenerator() {
	/// \todo Clean up vaChannel
}

void GlGenerator::setAnalyzer (std::shared_ptr<DSOAnalyzer::DataAnalyzer> analyzer) {
    dataAnalyzer = analyzer;
}

/// \brief Prepare arrays for drawing the data we get from the data analyzer.
    void GlGenerator::generateGraphs() {
	// Adapt the number of graphs
    qDebug() << "GlGenerator::generateGraphs: no of modes: " << DSO::ChannelMode::CHANNELMODE_COUNT << endl;
//    qDebug() << "format: " << this->settings->scope.horizontal.format << endl;

    for(int mode = DSO::ChannelMode::CHANNELMODE_VOLTAGE; mode < DSO::ChannelMode::CHANNELMODE_COUNT; ++mode) {
//        qDebug() << "vaChannel["<<mode<<"].count: " << this->vaChannel[mode].count() << endl;
		for(int channel = this->vaChannel[mode].count(); channel < this->settings->scope.voltage.count(); ++channel)
			this->vaChannel[mode].append(QList<GlArray *>());
		for(int channel = this->settings->scope.voltage.count(); channel < this->vaChannel[mode].count(); ++channel)
			this->vaChannel[mode].removeLast();
	}
//    qDebug() << "before digital phosphor" << endl;

    // Set digital phosphor depth to one if we don't use it
	if(this->settings->view.digitalPhosphor)
		this->digitalPhosphorDepth = this->settings->view.digitalPhosphorDepth;
	else
		this->digitalPhosphorDepth = 1;
	
	// Handle all digital phosphor related list manipulations
    for(int mode = DSO::ChannelMode::CHANNELMODE_VOLTAGE; mode < DSO::ChannelMode::CHANNELMODE_COUNT; ++mode) {
        for(int channel = 0; channel < this->vaChannel[mode].count(); ++channel) {
			// Resize lists for vector array if the digital phosphor depth has changed
			if(this->vaChannel[mode][channel].count() != this->digitalPhosphorDepth)
			for(int index = this->vaChannel[mode][channel].count(); index < this->digitalPhosphorDepth; ++index)
				this->vaChannel[mode][channel].append(new GlArray());
			for(int index = this->digitalPhosphorDepth; index < this->vaChannel[mode][channel].count(); ++index) {
				delete this->vaChannel[mode][channel].last();
				this->vaChannel[mode][channel].removeLast();
			}
			
			// Move the last list element to the front
			this->vaChannel[mode][channel].move(this->digitalPhosphorDepth -1, 0);
		}
    }
    int mode = DSO::ChannelMode::CHANNELMODE_VOLTAGE;
    unsigned int neededSize = 2*this->dataAnalyzer->sampleCount();

    switch(this->settings->scope.horizontal.format) {

    case DSO::GraphFormat::GRAPHFORMAT_TY:

        qDebug() << "GlGenerator::generateGraphs format TY" << endl;
        qDebug() << "this->settings->scope.voltage.count()" << this->settings->scope.voltage.count() <<endl;

        for (int i=0; i<this->settings->scope.voltage.count();i++) {

        if(this->settings->scope.voltage[i].used)
            qDebug() <<"channel "<<i<<" is used " << endl;
        else
            qDebug() <<"channel "<<i << " is not used " << endl;

        }

        qDebug() << "needed size " << this->dataAnalyzer->sampleCount() << endl;
        qDebug() << "no of channels " << this->settings->scope.voltage.count() << endl;

        for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
//        for(int channel = 0; channel < 2; ++channel) {
            if(!this->settings->scope.voltage[channel].used)
                continue;
            for(int index = 0; index < this->digitalPhosphorDepth; ++index) {
                if(this->vaChannel[mode][channel][index]->getSize() != neededSize)
                    this->vaChannel[mode][channel][index]->setSize(0);
            }

            for(int index = 0; index < this->digitalPhosphorDepth; ++index) {
                if(this->vaChannel[mode][channel][index]->getSize() != neededSize)
                    this->vaChannel[mode][channel][index]->setSize(0);
            }
            unsigned int arrayPosition = 0;
            // Check if the array is allocated
            if(!this->vaChannel[mode][channel].first()->data)
                this->vaChannel[mode][channel].first()->setSize(neededSize);

            GLfloat *vaNewChannel = this->vaChannel[mode][channel].first()->data;
            // What's the horizontal distance between sampling points?

//            qDebug() << "timebase" << this->settings->scope.horizontal.timebase << endl;

            const DSOAnalyzer::AnalyzedData *mydata= this->dataAnalyzer->data(channel);
            if (mydata == nullptr)
                break;
            const DSOAnalyzer::SampleValues& sampleValues = this->dataAnalyzer->data(channel)->samples.voltage;

            qDebug() << "sample rate: " << settings->scope.horizontal.samplerate << endl;
            qDebug() << "interval: " << sampleValues.interval <<" timebase: " << this->settings->scope.horizontal.timebase << endl;

            double horizontalFactor = sampleValues.interval / this->settings->scope.horizontal.timebase;
            qDebug() << "channel: " << channel << endl;
            qDebug() << " horizontalFactor: " << horizontalFactor;
            qDebug() << " sampleCount" << this->dataAnalyzer->sampleCount();
            qDebug() << " timebase" << settings->scope.horizontal.timebase;
            qDebug() << " DIVS_TIME" << DIVS_TIME << endl;

            qDebug() << "gain " <<  this->settings->scope.voltage[channel].gain;
            qDebug() << " gainID " << this->settings->scope.voltage[channel].gainID;
            qDebug() << " offset " <<  this->settings->scope.voltage[channel].offset << endl;

            double multFactor = this->settings->scope.voltage[channel].gainID / (double) this->settings->scope.voltage[channel].gain;
            qDebug() << "mult factor: " << multFactor  << endl;
            qDebug() << "sample count from analyzer: " << this->dataAnalyzer->sampleCount() << endl;
            for(unsigned int position = 0; position < this->dataAnalyzer->sampleCount(); ++position) {
                vaNewChannel[arrayPosition++] = position * horizontalFactor - (DIVS_TIME / 2);
                //                   vaNewChannel[arrayPosition++] = sin(position/50.)*4;
                //                  vaNewChannel[arrayPosition++] = sampleValues.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset;
                //vaNewChannel[arrayPosition++] = (sampleValues.sample[position]*4-2.0) /this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset;
                vaNewChannel[arrayPosition++] = (sampleValues.sample[position]-128)*multFactor/32.0+this->settings->scope.voltage[channel].offset;
            }
            /*
            if (channel == 2) {
                FILE *fd = fopen("/tmp/newsine.txt","w");
                for (int i=0;i<4000;i+=2)
                    fprintf(fd,"8.4%f %8.4f\n",vaNewChannel[i],vaNewChannel[i+1]);                 
                fclose(fd);

            if (channel == 2) {
                FILE *fd = fopen("/tmp/newsine.txt","w");
                for (int i=0;i<2000;i++)
                    fprintf(fd,"%f\n",sampleValues.sample[i]);
                fclose(fd);
            }
             */
        }
        break;

    default:
        break;
    }

/*
    switch(this->settings->scope.horizontal.format) {
    case Dso::GRAPHFORMAT_TY:
			// Add graphs for channels
            for(int mode = Dso::CHANNELMODE_VOLTAGE; mode < Dso::CHANNELMODE_COUNT; ++mode) {
                qDebug() << "generate graphs mode: " << mode << "Dso::CHANNELMODE_VOLTAGE" << Dso::CHANNELMODE_VOLTAGE << endl;
                if (mode != Dso::CHANNELMODE_VOLTAGE)
                    break;
				for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
					// Check if this channel is used and available at the data analyzer
                    if(((mode == Dso::CHANNELMODE_VOLTAGE) ? this->settings->scope.voltage[channel].used : this->settings->scope.spectrum[channel].used) ) {
//                  if(((mode == Dso::CHANNELMODE_VOLTAGE) ? this->settings->scope.voltage[channel].used : this->settings->scope.spectrum[channel].used) && this->dataAnalyzer->data(channel) && this->dataAnalyzer->data(channel)->samples.voltage.sample) {

                    // Check if the sample count has changed

//                        unsigned int neededSize = ((mode == Dso::CHANNELMODE_VOLTAGE) ? this->dataAnalyzer->data(channel)->samples.voltage.count : this->dataAnalyzer->data(channel)->samples.spectrum.count) * 2;
                        unsigned int neededSize = this->dataAnalyzer->sampleCount();
                        for(int index = 0; index < this->digitalPhosphorDepth; ++index) {
							if(this->vaChannel[mode][channel][index]->getSize() != neededSize)
								this->vaChannel[mode][channel][index]->setSize(0);
						}
						
						// Check if the array is allocated
						if(!this->vaChannel[mode][channel].first()->data)
							this->vaChannel[mode][channel].first()->setSize(neededSize);
						
						GLfloat *vaNewChannel = this->vaChannel[mode][channel].first()->data;
						
						// What's the horizontal distance between sampling points?

                        double horizontalFactor;

						if(mode == Dso::CHANNELMODE_VOLTAGE)
							horizontalFactor = this->dataAnalyzer->data(channel)->samples.voltage.interval / this->settings->scope.horizontal.timebase;
						else
							horizontalFactor = this->dataAnalyzer->data(channel)->samples.spectrum.interval / this->settings->scope.horizontal.frequencybase;

						// Fill vector array
						unsigned int arrayPosition = 0;
						if(mode == Dso::CHANNELMODE_VOLTAGE) {
                            for(unsigned int position = 0; position < this->dataAnalyzer->sampleCount(); ++position) {
								vaNewChannel[arrayPosition++] = position * horizontalFactor - DIVS_TIME / 2;
								vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset;
							}
						}
						else {
                            for(unsigned int position = 0; position < this->dataAnalyzer->sampleCount(); ++position) {
								vaNewChannel[arrayPosition++] = position * horizontalFactor - DIVS_TIME / 2;
								vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(channel)->samples.spectrum.sample[position] / this->settings->scope.spectrum[channel].magnitude + this->settings->scope.spectrum[channel].offset;
							}
						}
					}
					else {
						// Delete all vector arrays
						for(int index = 0; index < this->digitalPhosphorDepth; ++index)
							this->vaChannel[mode][channel][index]->setSize(0);
					}
				}
			}
			break;


		case Dso::GRAPHFORMAT_XY:
			for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
				// For even channel numbers check if this channel is used and this and the following channel are available at the data analyzer
//				if(channel % 2 == 0 && channel + 1 < this->settings->scope.voltage.count() && this->settings->scope.voltage[channel].used && this->dataAnalyzer->data(channel) && this->dataAnalyzer->data(channel)->samples.voltage.sample && this->dataAnalyzer->data(channel + 1) && this->dataAnalyzer->data(channel + 1)->samples.voltage.sample) {
                if(channel % 2 == 0 && channel + 1 < this->settings->scope.voltage.count() && this->settings->scope.voltage[channel].used) {
                    // Check if the sample count has changed
//					unsigned int neededSize = qMin(this->dataAnalyzer->data(channel)->samples.voltage.count, this->dataAnalyzer->data(channel + 1)->samples.voltage.count) * 2;
                    unsigned int neededSize = this->dataAnalyzer->sampleCount();
                    for(int index = 0; index < this->digitalPhosphorDepth; ++index) {
						if(this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->getSize() != neededSize)
							this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->setSize(0);
					}
					
					// Check if the array is allocated
					if(!this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->data)
						this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->setSize(neededSize);
					
					GLfloat *vaNewChannel = this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->data;
					
					// Fill vector array
					unsigned int arrayPosition = 0;
					unsigned int xChannel = channel;
					unsigned int yChannel = channel + 1;
                    for(unsigned int position = 0; position < this->dataAnalyzer->sampleCount(); ++position) {
						vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(xChannel)->samples.voltage.sample[position] / this->settings->scope.voltage[xChannel].gain + this->settings->scope.voltage[xChannel].offset;
						vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(yChannel)->samples.voltage.sample[position] / this->settings->scope.voltage[yChannel].gain + this->settings->scope.voltage[yChannel].offset;
					}
				}
				else {
					// Delete all vector arrays
					for(int index = 0; index < this->digitalPhosphorDepth; ++index)
						this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->setSize(0);
				}
				
				// Delete all spectrum graphs
				for(int index = 0; index < this->digitalPhosphorDepth; ++index)
					this->vaChannel[Dso::CHANNELMODE_SPECTRUM][channel][index]->setSize(0);
			}
			break;

		default:
			break;
	}
*/
//    qDebug() << "trigger mode: " << (int)DSO::TriggerMode::TRIGGERMODE_SINGLE << endl;
    if (settings->scope.trigger.mode == DSO::TriggerMode::TRIGGERMODE_SINGLE) {
        qDebug() << "emit singleShotFinish()" << endl;
        emit(singleShotFinish());
    }
    this->dataAnalyzer->mutex().unlock();

    emit graphsGenerated();

}


/// \brief Create the needed OpenGL vertex arrays for the grid.
void GlGenerator::generateGrid() {
	// Grid
	this->vaGrid[0].setSize(((DIVS_TIME * DIVS_SUB - 2) * (DIVS_VOLTAGE - 2) + (DIVS_VOLTAGE * DIVS_SUB - 2) * (DIVS_TIME - 2) - ((DIVS_TIME - 2) * (DIVS_VOLTAGE - 2))) * 2);
	int pointIndex = 0;
	// Draw vertical lines
	for(int div = 1; div < DIVS_TIME / 2; ++div) {
        for(int dot = 1; dot < DIVS_VOLTAGE / 2 * DIVS_SUB; ++dot ) { /// \brief Prepare arrays for drawing the data we get from the data analyzer.
			float dotPosition = (float) dot / DIVS_SUB;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
		}
	}
	// Draw horizontal lines
	for(int div = 1; div < DIVS_VOLTAGE / 2; ++div) {
		for(int dot = 1; dot < DIVS_TIME / 2 * DIVS_SUB; ++dot) {
			if(dot % DIVS_SUB == 0)
				continue;                   // Already done by vertical lines
			float dotPosition = (float) dot / DIVS_SUB;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
		}
	}
	
	// Axes
	this->vaGrid[1].setSize((2 + (DIVS_TIME * DIVS_SUB - 2) + (DIVS_VOLTAGE * DIVS_SUB - 2)) * 4);
	pointIndex = 0;
	// Horizontal axis
	this->vaGrid[1].data[pointIndex++] = -DIVS_TIME / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = DIVS_TIME / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	// Vertical axis
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = -DIVS_VOLTAGE / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = DIVS_VOLTAGE / 2;
	// Subdiv lines on horizontal axis
	for(int line = 1; line < DIVS_TIME / 2 * DIVS_SUB; ++line) {
		float linePosition = (float) line / DIVS_SUB;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
	}
	// Subdiv lines on vertical axis
	for(int line = 1; line < DIVS_VOLTAGE / 2 * DIVS_SUB; ++line) {
		float linePosition = (float) line / DIVS_SUB;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
	}
	
	// Border
	this->vaGrid[2].setSize(4 * 2);
	this->vaGrid[2].data[0] = -DIVS_TIME / 2;
	this->vaGrid[2].data[1] = -DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[2] = DIVS_TIME / 2;
	this->vaGrid[2].data[3] = -DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[4] = DIVS_TIME / 2;
	this->vaGrid[2].data[5] = DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[6] = -DIVS_TIME / 2;
	this->vaGrid[2].data[7] = DIVS_VOLTAGE / 2;
}

