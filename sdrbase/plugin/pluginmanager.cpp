#include <QApplication>
#include <QPluginLoader>
#include <QComboBox>
#include "plugin/pluginmanager.h"
#include "plugin/plugingui.h"
#include "settings/preset.h"
#include "mainwindow.h"
#include "dsp/dspengine.h"
#include "dsp/samplesource.h"

#include <QDebug>
#include "util/stacktrace.h"

PluginManager::PluginManager(MainWindow* mainWindow, DSPEngine* dspEngine, QObject* parent) :
	QObject(parent),
	m_pluginAPI(this, mainWindow, dspEngine),
	m_mainWindow(mainWindow),
	m_dspEngine(dspEngine),
	m_sampleSourceId(),
	m_sampleSourceSerial(),
	m_sampleSourceSequence(0),
	m_sampleSourcePluginGUI(NULL)
{
}

PluginManager::~PluginManager()
{
	freeAll();
}

void PluginManager::loadPlugins()
{
	qDebug() << "PluginManager::loadPlugins: " << qPrintable(QApplication::instance()->applicationDirPath());

	QDir pluginsDir = QDir(QApplication::instance()->applicationDirPath());

	loadPlugins(pluginsDir);

	qSort(m_plugins);

	for (Plugins::const_iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
	{
		it->pluginInterface->initPlugin(&m_pluginAPI);
	}

	updateSampleSourceDevices();
}

void PluginManager::registerChannel(const QString& channelName, PluginInterface* plugin, QAction* action)
{
	m_channelRegistrations.append(ChannelRegistration(channelName, plugin));
	m_mainWindow->addChannelCreateAction(action);
}

void PluginManager::registerChannelInstance(const QString& channelName, PluginGUI* pluginGUI)
{
	m_channelInstanceRegistrations.append(ChannelInstanceRegistration(channelName, pluginGUI));
	renameChannelInstances();
}

void PluginManager::addChannelRollup(QWidget* pluginGUI)
{
	m_mainWindow->addChannelRollup(pluginGUI);
}

void PluginManager::removeChannelInstance(PluginGUI* pluginGUI)
{
	for(ChannelInstanceRegistrations::iterator it = m_channelInstanceRegistrations.begin(); it != m_channelInstanceRegistrations.end(); ++it) {
		if(it->m_gui == pluginGUI) {
			m_channelInstanceRegistrations.erase(it);
			break;
		}
	}
	renameChannelInstances();
}

void PluginManager::registerSampleSource(const QString& sourceName, PluginInterface* plugin)
{
	qDebug() << "PluginManager::registerSampleSource "
			<< plugin->getPluginDescriptor().displayedName.toStdString().c_str()
			<< " with source name " << sourceName.toStdString().c_str();

	m_sampleSourceRegistrations.append(SampleSourceRegistration(sourceName, plugin));
}

void PluginManager::loadSettings(const Preset* preset)
{
	fprintf(stderr, "PluginManager::loadSettings: Loading preset [%s | %s]\n", qPrintable(preset->getGroup()), qPrintable(preset->getDescription()));

	// copy currently open channels and clear list
	ChannelInstanceRegistrations openChannels = m_channelInstanceRegistrations;
	m_channelInstanceRegistrations.clear();

	for(int i = 0; i < preset->getChannelCount(); i++)
	{
		const Preset::ChannelConfig& channelConfig = preset->getChannelConfig(i);
		ChannelInstanceRegistration reg;

		// if we have one instance available already, use it

		for(int i = 0; i < openChannels.count(); i++)
		{
			qDebug("PluginManager::loadSettings: channels compare [%s] vs [%s]", qPrintable(openChannels[i].m_channelName), qPrintable(channelConfig.m_channel));

			if(openChannels[i].m_channelName == channelConfig.m_channel)
			{
				qDebug("PluginManager::loadSettings: channel [%s] found", qPrintable(openChannels[i].m_channelName));
				reg = openChannels.takeAt(i);
				m_channelInstanceRegistrations.append(reg);
				break;
			}
		}

		// if we haven't one already, create one

		if(reg.m_gui == NULL)
		{
			for(int i = 0; i < m_channelRegistrations.count(); i++)
			{
				if(m_channelRegistrations[i].m_channelName == channelConfig.m_channel)
				{
					qDebug("PluginManager::loadSettings: creating new channel [%s]", qPrintable(channelConfig.m_channel));
					reg = ChannelInstanceRegistration(channelConfig.m_channel, m_channelRegistrations[i].m_plugin->createChannel(channelConfig.m_channel));
					break;
				}
			}
		}

		if(reg.m_gui != NULL)
		{
			qDebug("PluginManager::loadSettings: deserializing channel [%s]", qPrintable(channelConfig.m_channel));
			reg.m_gui->deserialize(channelConfig.m_config);
		}
	}

	// everything, that is still "available" is not needed anymore
	for(int i = 0; i < openChannels.count(); i++)
	{
		qDebug("PluginManager::loadSettings: destroying spare channel [%s]", qPrintable(openChannels[i].m_channelName));
		openChannels[i].m_gui->destroy();
	}

	renameChannelInstances();

	if(m_sampleSourcePluginGUI != 0)
	{
		qDebug("PluginManager::loadSettings: source compare [%s] vs [%s]", qPrintable(m_sampleSourceId), qPrintable(preset->getSourceId()));

		// TODO: have one set of source presets per identified source (preset -> find source with name)
		if(m_sampleSourceId == preset->getSourceId())
		{
			qDebug() << "PluginManager::loadSettings: deserializing source " << qPrintable(m_sampleSourceId);
			m_sampleSourcePluginGUI->deserialize(preset->getSourceConfig());
		}

		// FIXME: get center frequency from preset center frequency
		qint64 centerFrequency = preset->getCenterFrequency();
		m_sampleSourcePluginGUI->setCenterFrequency(centerFrequency);
	}
}

// sort by increasing delta frequency and type (i.e. name)
bool PluginManager::ChannelInstanceRegistration::operator<(const ChannelInstanceRegistration& other) const {
	if (m_gui && other.m_gui) {
		if (m_gui->getCenterFrequency() == other.m_gui->getCenterFrequency()) {
			return m_gui->getName() < other.m_gui->getName();
		} else {
			return m_gui->getCenterFrequency() < other.m_gui->getCenterFrequency();
		}
	} else {
		return false;
	}
}

void PluginManager::saveSettings(Preset* preset)
{
	if(m_sampleSourcePluginGUI != NULL)
	{
		preset->setSourceConfig(m_sampleSourceId, m_sampleSourceSerial, m_sampleSourceSequence, m_sampleSourcePluginGUI->serialize());
		preset->setCenterFrequency(m_sampleSourcePluginGUI->getCenterFrequency());
	}
	else
	{
		preset->setSourceConfig(QString::null, QString::null, 0, QByteArray());
	}

	qSort(m_channelInstanceRegistrations.begin(), m_channelInstanceRegistrations.end()); // sort by increasing delta frequency and type

	for(int i = 0; i < m_channelInstanceRegistrations.count(); i++)
	{
		preset->addChannel(m_channelInstanceRegistrations[i].m_channelName, m_channelInstanceRegistrations[i].m_gui->serialize());
	}
}

void PluginManager::freeAll()
{
	m_dspEngine->stopAcquistion();

	while(!m_channelInstanceRegistrations.isEmpty()) {
		ChannelInstanceRegistration reg(m_channelInstanceRegistrations.takeLast());
		reg.m_gui->destroy();
	}

	if(m_sampleSourcePluginGUI != NULL) {
		m_dspEngine->setSource(NULL);
		m_sampleSourcePluginGUI->destroy();
		m_sampleSourcePluginGUI = NULL;
		m_sampleSourceId.clear();
	}
}

bool PluginManager::handleMessage(const Message& message)
{
	if (m_sampleSourcePluginGUI != 0)
	{
		if ((message.getDestination() == 0) || (message.getDestination() == m_sampleSourcePluginGUI))
		{
			if (m_sampleSourcePluginGUI->handleMessage(message))
			{
				return true;
			}
		}
	}

	for (ChannelInstanceRegistrations::iterator it = m_channelInstanceRegistrations.begin(); it != m_channelInstanceRegistrations.end(); ++it)
	{
		if ((message.getDestination() == 0) || (message.getDestination() == it->m_gui))
		{
			if (it->m_gui->handleMessage(message))
			{
				return true;
			}
		}
	}

	return false;
}

void PluginManager::updateSampleSourceDevices()
{
	m_sampleSourceDevices.clear();

	for(int i = 0; i < m_sampleSourceRegistrations.count(); ++i)
	{
		PluginInterface::SampleSourceDevices ssd = m_sampleSourceRegistrations[i].m_plugin->enumSampleSources();

		for(int j = 0; j < ssd.count(); ++j)
		{
			m_sampleSourceDevices.append(SampleSourceDevice(m_sampleSourceRegistrations[i].m_plugin,
					ssd[j].displayedName,
					ssd[j].id,
					ssd[j].serial,
					ssd[j].sequence));
		}
	}
}

void PluginManager::fillSampleSourceSelector(QComboBox* comboBox)
{
	comboBox->clear();
	for(int i = 0; i < m_sampleSourceDevices.count(); i++)
		comboBox->addItem(m_sampleSourceDevices[i].m_displayName, i);
}

int PluginManager::selectSampleSource(int index)
{
	qDebug() << "PluginManager::selectSampleSource by index";

	m_dspEngine->stopAcquistion();

	if(m_sampleSourcePluginGUI != NULL) {
		m_dspEngine->stopAcquistion();
		m_dspEngine->setSource(NULL);
		m_sampleSourcePluginGUI->destroy();
		m_sampleSourcePluginGUI = NULL;
		m_sampleSourceId.clear();
	}

	if(index == -1)
	{
		if(!m_sampleSourceId.isEmpty())
		{
			for(int i = 0; i < m_sampleSourceDevices.count(); i++)
			{
				if(m_sampleSourceDevices[i].m_sourceId == m_sampleSourceId)
				{
					index = i;
					break;
				}
			}
		}

		if(index == -1)
		{
			if(m_sampleSourceDevices.count() > 0)
			{
				index = 0;
			}
		}
	}

	if(index == -1)
	{
		return -1;
	}

	m_sampleSourceId = m_sampleSourceDevices[index].m_sourceId;
	m_sampleSourceSerial = m_sampleSourceDevices[index].m_sourceSerial;
	m_sampleSourceSequence = m_sampleSourceDevices[index].m_sourceSequence;

	qDebug() << "m_sampleSource at index " << index
			<< " id: " << m_sampleSourceId.toStdString().c_str()
			<< " ser: " << m_sampleSourceSerial.toStdString().c_str()
			<< " seq: " << m_sampleSourceSequence;

	m_sampleSourcePluginGUI = m_sampleSourceDevices[index].m_plugin->createSampleSourcePluginGUI(m_sampleSourceId);

	return index;
}

int PluginManager::selectFirstSampleSource(const QString& sourceId)
{
	qDebug() << "PluginManager::selectFirstSampleSource by id: " << sourceId.toStdString().c_str();

	int index = -1;

	m_dspEngine->stopAcquistion();

	if(m_sampleSourcePluginGUI != NULL) {
		m_dspEngine->stopAcquistion();
		m_dspEngine->setSource(NULL);
		m_sampleSourcePluginGUI->destroy();
		m_sampleSourcePluginGUI = NULL;
		m_sampleSourceId.clear();
	}

	qDebug("finding first sample source [%s]", qPrintable(sourceId));

	for(int i = 0; i < m_sampleSourceDevices.count(); i++)
	{
		qDebug("*** %s vs %s", qPrintable(m_sampleSourceDevices[i].m_sourceId), qPrintable(sourceId));

		if(m_sampleSourceDevices[i].m_sourceId == sourceId)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
	{
		if(m_sampleSourceDevices.count() > 0)
		{
			index = 0;
		}
	}

	if(index == -1)
	{
		return -1;
	}

	m_sampleSourceId = m_sampleSourceDevices[index].m_sourceId;
	m_sampleSourceSerial = m_sampleSourceDevices[index].m_sourceSerial;
	m_sampleSourceSequence = m_sampleSourceDevices[index].m_sourceSequence;

	qDebug() << "m_sampleSource at index " << index
			<< " id: " << m_sampleSourceId.toStdString().c_str()
			<< " ser: " << m_sampleSourceSerial.toStdString().c_str()
			<< " seq: " << m_sampleSourceSequence;

	m_sampleSourcePluginGUI = m_sampleSourceDevices[index].m_plugin->createSampleSourcePluginGUI(m_sampleSourceId);

	return index;
}

void PluginManager::loadPlugins(const QDir& dir)
{
	QDir pluginsDir(dir);

	foreach (QString fileName, pluginsDir.entryList(QDir::Files))
	{
		if (fileName.endsWith(".so"))
		{
			qDebug() << "PluginManager::loadPlugins: fileName: " << qPrintable(fileName);

			QPluginLoader* loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
			PluginInterface* plugin = qobject_cast<PluginInterface*>(loader->instance());

			if (loader->isLoaded())
			{
				qWarning("PluginManager::loadPlugins: loaded plugin %s", qPrintable(fileName));
			}
			else
			{
				qWarning() << "PluginManager::loadPlugins: " << qPrintable(loader->errorString());
			}

			if (plugin != 0)
			{
				m_plugins.append(Plugin(fileName, loader, plugin));
			}
			else
			{
				loader->unload();
			}

			delete loader; // Valgrind memcheck
		}
	}

	// recursive calls on subdirectories

	foreach (QString dirName, pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
	{
		loadPlugins(pluginsDir.absoluteFilePath(dirName));
	}
}

void PluginManager::renameChannelInstances()
{
	for(int i = 0; i < m_channelInstanceRegistrations.count(); i++) {
		m_channelInstanceRegistrations[i].m_gui->setName(QString("%1:%2").arg(m_channelInstanceRegistrations[i].m_channelName).arg(i));
	}
}
