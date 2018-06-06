#ifndef QTSERVICE_PLUGIN_H
#define QTSERVICE_PLUGIN_H

#include <QQmlExtensionPlugin>

class QtServiceDeclarativeModule : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
	QtServiceDeclarativeModule(QObject *parent = nullptr);
	void registerTypes(const char *uri) override;
};

#endif // QTSERVICE_PLUGIN_H
