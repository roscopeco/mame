#ifndef __DEBUG_QT_TERM_WINDOW_H__
#define __DEBUG_QT_TERM_WINDOW_H__

#include "debuggerview.h"
#include "windowqt.h"


class QTermWidget;
//============================================================
//  The Terminal Window.
//============================================================
class TermWindow : public WindowQt
{
	Q_OBJECT

public:
	TermWindow(running_machine* machine, QWidget* parent=nullptr);
	virtual ~TermWindow();


private:
	// Widgets
	QTermWidget *console;
};


//=========================================================================
//  A way to store the configuration of a window long enough to read/write.
//=========================================================================
class TermWindowQtConfig : public WindowQtConfig
{
public:
	TermWindowQtConfig() :
		WindowQtConfig(WIN_TYPE_TERM)
	{
	}

	~TermWindowQtConfig() {}

	void buildFromQWidget(QWidget* widget);
	void applyToQWidget(QWidget* widget);
	void addToXmlDataNode(util::xml::data_node &node) const;
	void recoverFromXmlNode(util::xml::data_node const &node);
};


#endif
