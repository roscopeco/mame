#include "emu.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QApplication>

#include "termwindow.h"
#include "qtermwidget/lib/qtermwidget.h"
#include <unistd.h>
static QTermWidget *main_console = nullptr;
void osd_term_write(uint8_t data)
{
	if (main_console)
		write(main_console->getPtySlaveFd(), &data, 1);
}

extern void osd_send_key(uint8_t key);

TermWindow::TermWindow(running_machine* machine, QWidget* parent) :
	WindowQt(machine, nullptr)
{
	setWindowTitle("Terminal");

	if (parent != nullptr)
	{
		QPoint parentPos = parent->pos();
		setGeometry(parentPos.x()+100, parentPos.y()+100, 800, 400);
	}

	//
	// The main frame and its input and log widgets
	//
	console = new QTermWidget(0);
	main_console = console;
	QFont font = QApplication::font();
	#ifdef Q_OS_MACOS
		font.setFamily(QStringLiteral("Monaco"));
	#elif defined(Q_WS_QWS)
		font.setFamily(QStringLiteral("fixed"));
	#else
		font.setFamily(QStringLiteral("Monospace"));
	#endif
		font.setPointSize(12);

	console->setTerminalFont(font);
	//console->setColorScheme("DarkPastels");
	console->setColorScheme("Linux");
	console->setScrollBarPosition(QTermWidget::ScrollBarRight);
   	console->startTerminalTeletype();
	// Write what we input to remote terminal via socket
	connect(console, &QTermWidget::sendData,[](const char *data, int size){
		for(int i=0;i<size;i++) {
			osd_send_key(data[i]);
		}
		
	});

	setCentralWidget(console);
}


TermWindow::~TermWindow()
{
}


//=========================================================================
//  TermWindowQtConfig
//=========================================================================
void TermWindowQtConfig::buildFromQWidget(QWidget* widget)
{
	WindowQtConfig::buildFromQWidget(widget);
}


void TermWindowQtConfig::applyToQWidget(QWidget* widget)
{
	WindowQtConfig::applyToQWidget(widget);
}


void TermWindowQtConfig::addToXmlDataNode(util::xml::data_node &node) const
{
	WindowQtConfig::addToXmlDataNode(node);
}


void TermWindowQtConfig::recoverFromXmlNode(util::xml::data_node const &node)
{
	WindowQtConfig::recoverFromXmlNode(node);
}
