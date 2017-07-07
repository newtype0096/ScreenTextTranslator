#pragma once

#include <QtWidgets/QMainWindow>
#include <qsystemtrayicon.h>
#include <qmenu.h>
#include <qdatetime.h>
#include <atlstr.h>
#include <tinyxml2/tinyxml2.h>
#include "ui_ScreenTextTranslator.h"
#include "ui_About.h" // Form_About
#include "ui_Settings.h" // Form_Settings
#include "ui_TransMode.h" // TransMode
#include "TessCV.h"
#include "AzureAPI.h"

class ScreenTextTranslator : public QMainWindow
{
	Q_OBJECT

public:
	ScreenTextTranslator(QWidget *parent = Q_NULLPTR);
	QWidget *Form_About; // Form_About
	QWidget *Form_Settings; // Form_Settings
	QWidget *TransMode; // TransMode
	
	void startOCR();
	void getTransText();
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);

	TessCV *tesscv = new TessCV;
	Azure *azure = new Azure;
	QSystemTrayIcon *tray;
	QMenu *traymenu;

private slots:
	void openForm_About(); // About �� ����
	void openForm_Settings(); // Settings �� ����
	void selectROI(); // �׽�Ʈ ���� ����
	void testTrans(); // ���� ����
	void addKey(); // ���� Ű ���
	void ChangeTransMode(); // ���� ��� ��ȯ
	void BackToMain(); // ���� ������� ���ư�
	void SelectTransROI(); // ���� ���� ����
	void StartTrans(); // ���� ����
	void ShowROI();

private:
	Ui::ScreenTextTranslatorClass ui;
	Ui::Form_About fa; // Form_About
	Ui::Form_Settings fs; // Form_Settings
	Ui::Form_TransMode ft; // TransMode
};