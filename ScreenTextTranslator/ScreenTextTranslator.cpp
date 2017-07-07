#include "ScreenTextTranslator.h"

namespace STT
{
	// ���콺 �̺�Ʈ �ڵ�
	Point point1, point2;
	Rect rect;

	// ����ȭ�� �̹���
	Mat img;

	// ���콺 �巡�� ����
	int drag = 0;

	// ���� ���� �Ϸ� ����
	int select_flag = 0;

	// OCR �ؽ�Ʈ
	utility::string_t ocrText;

	// Azure ���� Ű
	utility::string_t AzureKey;

	// ��ū ���� ��� �ð�
	QTime TokenTime;

	// ��ū ��ȿ �ð�
	QTime TokenLimit(0,8);

	// ���� ����
	bool bToken = false;

	// ���� �ؽ�Ʈ
	utility::string_t transText;

	// ������ ���� ��� ����
	bool bShowROI = false;
}

void mouseHandler(int event, int x, int y, int flags, void *param);

ScreenTextTranslator::ScreenTextTranslator(QWidget *parent)
	: QMainWindow(parent)
{	
	ui.setupUi(this);
	this->setContextMenuPolicy(Qt::NoContextMenu); // Toolbar ��Ŭ���� ������ ���ؽ�Ʈ �޴� ��Ȱ��ȭ
	connect(ui.actionInfo, SIGNAL(triggered()), this, SLOT(openForm_About())); // actionInfo ��ư�� �׼� ����
	connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(openForm_Settings())); // actionSettings ��ư�� �׼� ����
	connect(ui.actionOcrTest, SIGNAL(triggered()), this, SLOT(selectROI())); // actionOcrTest ��ư�� �׼� ����
	connect(ui.actionTransTextTest, SIGNAL(triggered()), this, SLOT(testTrans())); // actionOcrTest ��ư�� �׼� ����
	connect(ui.actionMode, SIGNAL(triggered()), this, SLOT(ChangeTransMode())); // actionMode ��ư�� �׼� ����

	// Toolbar ��Ŭ���� ��Ÿ���� ���ؽ�Ʈ �޴� ��Ȱ��ȭ
	this->setContextMenuPolicy(Qt::NoContextMenu); 

	// settings.ini ���� �б�
	TCHAR tempKey[35];
	GetPrivateProfileString(TEXT("Azure ���� Ű"), TEXT("Subscription Key"), NULL, tempKey, sizeof(tempKey), TEXT("./settings.ini"));
	std::string _tempKey = CW2A(tempKey);
	STT::AzureKey = utility::conversions::to_string_t(_tempKey);

	// Ʈ���� ������
	QIcon icon;
	icon.addFile(QStringLiteral(":/ScreenTextTranslator/Logo.jpg"), QSize(), QIcon::Normal, QIcon::Off);

	// Ʈ���� �޴�
	traymenu = new QMenu(this);
	traymenu->addAction(ui.actionSettings);
	traymenu->addSeparator();
	traymenu->addAction(ui.actionInfo);
	traymenu->addSeparator();
	traymenu->addAction(ui.actionExit);

	// Ʈ����
	tray = new QSystemTrayIcon(this);
	tray->setIcon(icon);
	tray->setToolTip(QString::fromLocal8Bit("ȭ�� �ؽ�Ʈ ������ (Screen Text Translator)"));
	tray->setContextMenu(traymenu);
	tray->show();
}

// About �� ����
void ScreenTextTranslator::openForm_About() 
{
	Form_About = new QWidget;
	fa.setupUi(Form_About);
	Form_About->setWindowFlags(Qt::WindowCloseButtonHint); // ������ �ּ�ȭ, �ִ�ȭ ��ư ����
	Form_About->show();
}

// Settings �� ����
void ScreenTextTranslator::openForm_Settings() 
{
	Form_Settings = new QWidget;
	fs.setupUi(Form_Settings);
	Form_Settings->setWindowFlags(Qt::WindowCloseButtonHint); // ������ �ּ�ȭ, �ִ�ȭ ��ư ����
	Form_Settings->show();

	fs.lineEdit_SubscriptionKey->setText(QString::fromStdString(utility::conversions::to_utf8string(STT::AzureKey)));

	connect(fs.pushButton_OK, SIGNAL(clicked()), this, SLOT(addKey())); // ���� - Ȯ�� ��ư�� �׼� ����
}

// ���� ��� ��ȯ
void ScreenTextTranslator::ChangeTransMode()
{
	this->hide();

	TransMode = new QWidget;
	ft.setupUi(TransMode);
	TransMode->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint); // ������ Ÿ��Ʋ�� ��� ��ư ����
	TransMode->show();

	connect(ft.pushButton_Back, SIGNAL(clicked()), this, SLOT(BackToMain())); // ���ư��� ��ư�� �׼� ����
	connect(ft.toolButton_Settings, SIGNAL(clicked()), this, SLOT(openForm_Settings())); // ���� ��ư�� �׼� ����
	connect(ft.toolButton_SelectROI, SIGNAL(clicked()), this, SLOT(SelectTransROI())); // ���� ���� ��ư�� �׼� ����
	connect(ft.toolButton_TransStart, SIGNAL(clicked()), this, SLOT(StartTrans())); // ���� ���� ��ư�� �׼� ����
	connect(ft.toolButton_ShowROI, SIGNAL(clicked()), this, SLOT(ShowROI())); // ���� ���� ��ư�� �׼� ����

	RegisterHotKey((HWND)this->winId(), 0, MOD_SHIFT, 'A');
	RegisterHotKey((HWND)this->winId(), 1, MOD_SHIFT, 'S');
	RegisterHotKey((HWND)this->winId(), 2, MOD_SHIFT, 'D');
}

// ���α׷� ���� ����Ű
bool ScreenTextTranslator::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

	MSG *msg = static_cast<MSG*>(message);
	if (msg->message == WM_HOTKEY)
	{
		if (msg->wParam == 0)
		{
			//MessageBox(NULL, TEXT("TRANS OK"), TEXT("OK"), MB_OK);
			ft.toolButton_TransStart->clicked();
			return true;
		}
		else if (msg->wParam == 1)
		{
			//MessageBox(NULL, TEXT("SELECT OK"), TEXT("OK"), MB_OK);
			ft.toolButton_SelectROI->clicked();
			return true;
		}
		else if (msg->wParam == 2)
		{
			//MessageBox(NULL, TEXT("ROI OK"), TEXT("OK"), MB_OK);
			ft.toolButton_ShowROI->clicked();
			return true;
		}
	}
	return false;
}

// ������ ���� ���
void ScreenTextTranslator::ShowROI()
{
	//STT::bShowROI = !STT::bShowROI;

	//if (STT::bShowROI)
	//{
		HDC hdc = GetDC(0);
		Rectangle(hdc, STT::point1.x, STT::point1.y, STT::point2.x, STT::point2.y);
	//}
	//else
	//{

	//}
}

// ���� ����
void ScreenTextTranslator::StartTrans()
{
	tesscv->getScreen(STT::img);

	Mat roi = STT::img(STT::rect);

	try
	{
		cv::resize(roi, roi, Size(roi.cols * 3, roi.rows * 3), 0, 0, CV_INTER_NN);

		cvtColor(roi, roi, CV_BGR2GRAY);
		threshold(roi, roi, 130, 255, THRESH_BINARY);

		std::string tempText = tesscv->getText(roi);
		STT::ocrText = utility::conversions::to_string_t(tempText);
	}
	catch (Exception ex)
	{
		MessageBox(NULL, TEXT("������ �ٽ� �����ϼ���"), TEXT("���� ���� ����"), MB_OK | MB_ICONEXCLAMATION);
		STT::select_flag = 0;
		SelectTransROI();
	}

	utility::string_t space = utility::conversions::to_string_t("");

	// ocrText ���� �˻�
	if (space != STT::ocrText)
	{

		// ù ��ū ����
		if (STT::bToken == false)
		{
			QTime currentTime;
			STT::TokenTime = currentTime.currentTime();
			STT::bToken = true;

			azure->Init(STT::AzureKey);
			azure->GetAccessTokenAsync(STT::bToken).wait();
			azure->GetTranslateText(STT::ocrText, STT::transText, STT::bToken).wait();

			if (STT::bToken)
			{
				ft.plainTextEdit_TransText->setPlainText(QString::fromStdString(utility::conversions::to_utf8string(STT::transText)).append(QString::fromStdString(utility::conversions::to_utf8string(STT::ocrText))));
			}
			else
			{
				std::string error = "API ���� Ű�� �߸� �Ǿ��ų� ����Ǿ����ϴ�.";
				ft.plainTextEdit_TransText->setPlainText(QString::fromLocal8Bit(error.c_str()).append(QString::fromStdString(utility::conversions::to_utf8string(STT::ocrText))));
			}			
		}
		
		// ��ū ���� ��
		else if (STT::bToken == true)
		{
			QTime currentTime = currentTime.currentTime();

			// ��ū ���� �ð� 8�� �˻�, 8�� �ʰ��� ��ū �� ����
			if (currentTime.minute() - STT::TokenTime.minute() >= 0)
			{
				if (currentTime.minute() - STT::TokenTime.minute() > STT::TokenLimit.minute())
				{
					azure->GetAccessTokenAsync(STT::bToken).wait();
				}
			}
			else
			{
				if (currentTime.minute() - STT::TokenTime.minute() < STT::TokenLimit.minute())
				{
					azure->GetAccessTokenAsync(STT::bToken).wait();
				}
			}

			azure->GetTranslateText(STT::ocrText, STT::transText, STT::bToken).wait();

			if (STT::bToken)
			{
				ft.plainTextEdit_TransText->setPlainText(QString::fromStdString(utility::conversions::to_utf8string(STT::transText)).append(QString::fromStdString(utility::conversions::to_utf8string(STT::ocrText))));
			}
			else
			{
				std::string error = "API ���� Ű�� �߸� �Ǿ��ų� ����Ǿ����ϴ�.";
				ft.plainTextEdit_TransText->setPlainText(QString::fromLocal8Bit(error.c_str()).append(QString::fromStdString(utility::conversions::to_utf8string(STT::ocrText))));
			}
		}

	}

	//SwitchToThisWindow((HWND)TransMode->winId(), TRUE);
}

// ���� ���� ����
void ScreenTextTranslator::SelectTransROI()
{
	STT::select_flag = 0;

	TransMode->hide();

	Sleep(200);

	tesscv->getScreen(STT::img);

	namedWindow("���� ����", WINDOW_NORMAL);
	setWindowProperty("���� ����", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	imshow("���� ����", STT::img);

	HWND cvhwnd = (HWND)cvGetWindowHandle("���� ����");
	SwitchToThisWindow(cvhwnd, TRUE);

	cvSetMouseCallback("���� ����", mouseHandler, NULL); // ���� ���� ������ ���
	
	int key;

	while (IsWindowVisible(cvhwnd)) // ���� ���� �����찡 �����ϸ�
	{
		key = waitKey(60);

		if (key == 27)
		{
			cvDestroyWindow("���� ����");
			STT::select_flag = 0;
			break; // ������ ������ Ż��
		}

		if (STT::select_flag == 1)
		{
			cvDestroyWindow("���� ����");
			break; // ������ ������ Ż��
		}
	}

	TransMode->show();
}

// ���� ������� ���ư�
void ScreenTextTranslator::BackToMain()
{
	UnregisterHotKey((HWND)this->winId(), 0);
	UnregisterHotKey((HWND)this->winId(), 1);
	UnregisterHotKey((HWND)this->winId(), 2);
	TransMode->close();
	this->show();
}

// �׽�Ʈ ���� ����
void ScreenTextTranslator::selectROI()
{
	STT::select_flag = 0;

	this->hide();

	Sleep(200);

	tesscv->getScreen(STT::img);

	namedWindow("���� ����", WINDOW_NORMAL);
	setWindowProperty("���� ����", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	imshow("���� ����", STT::img);

	HWND cvhwnd = (HWND)cvGetWindowHandle("���� ����");

	cvSetMouseCallback("���� ����", mouseHandler, NULL); // ���� ���� ������ ���

	int key;

	while (IsWindowVisible(cvhwnd)) // ���� ���� �����찡 �����ϸ�
	{
		key = waitKey(60);

		if (key == 27)
		{
			cvDestroyWindow("���� ����");
			STT::select_flag = 0;
			break; // ������ ������ Ż��
		}

		if (STT::select_flag == 1)
		{
			cvDestroyWindow("���� ����");			
			break; // ������ ������ Ż��
		}
	}

	this->show();

	if (STT::select_flag == 1)
	{
		startOCR();
	}
}

// ���� �ν�
void ScreenTextTranslator::startOCR()
{
	Mat roi = STT::img(STT::rect);

	try
	{
		cv::resize(roi, roi, Size(roi.cols * 3, roi.rows * 3), 0, 0, CV_INTER_NN);

		cvtColor(roi, roi, CV_BGR2GRAY);
		threshold(roi, roi, 130, 255, THRESH_BINARY);

		// �����ν� �� �ϴ� Plain Text�� ���
		std::string tempText = tesscv->getText(roi);
		STT::ocrText = utility::conversions::to_string_t(tempText);
		ui.textBrowser_OCR->setPlainText(QString::fromStdString(tempText));
	}
	catch (Exception ex)
	{
		MessageBox(NULL, TEXT("������ �ٽ� �����ϼ���"), TEXT("���� ���� ����"), MB_OK | MB_ICONEXCLAMATION);
		STT::select_flag = 0;
		selectROI();
	}
}

// Azure ���� Ű ���
void ScreenTextTranslator::addKey()
{
	STT::AzureKey = utility::conversions::to_string_t(fs.lineEdit_SubscriptionKey->text().toStdString());

	// settings.ini ���Ͽ� ���
	WritePrivateProfileString(TEXT("Azure ���� Ű"), TEXT("Subscription Key"), fs.lineEdit_SubscriptionKey->text().toStdWString().c_str(), TEXT("./settings.ini"));

	Form_Settings->close();
}

// ���� �׽�Ʈ ����
void ScreenTextTranslator::testTrans()
{
	getTransText();
}

// ������ ���
void ScreenTextTranslator::getTransText()
{
	utility::string_t space = utility::conversions::to_string_t("");

	if (space != STT::ocrText)
	{

		if (STT::bToken == false)
		{
			QTime currentTime;
			STT::TokenTime = currentTime.currentTime();
			STT::bToken = true;

			azure->Init(STT::AzureKey);
			azure->GetAccessTokenAsync(STT::bToken).wait();
			azure->GetTranslateText(STT::ocrText, STT::transText, STT::bToken).wait();

			
			if (STT::bToken)
			{
				ui.textBrowser_Trans->setPlainText(QString::fromStdString(utility::conversions::to_utf8string(STT::transText)));
			}
			// API �Ұ� ��
			else
			{
				std::string error = "API ���� Ű�� �߸� �Ǿ��ų� ����Ǿ����ϴ�.";
				ui.textBrowser_Trans->setPlainText(QString::fromLocal8Bit(error.c_str()));
			}
		}
		else if (STT::bToken == true)
		{
			QTime currentTime = currentTime.currentTime();

			if (currentTime.minute() - STT::TokenTime.minute() >= 0)
			{
				if (currentTime.minute() - STT::TokenTime.minute() > STT::TokenLimit.minute())
				{
					azure->GetAccessTokenAsync(STT::bToken).wait();
				}
			}
			else
			{
				if (currentTime.minute() - STT::TokenTime.minute() < STT::TokenLimit.minute())
				{
					azure->GetAccessTokenAsync(STT::bToken).wait();
				}
			}

			azure->GetTranslateText(STT::ocrText, STT::transText, STT::bToken).wait();

			if (STT::bToken)
			{
				ui.textBrowser_Trans->setPlainText(QString::fromStdString(utility::conversions::to_utf8string(STT::transText)));
			}
			// API �Ұ� ��
			else
			{
				std::string error = "API ���� Ű�� �߸� �Ǿ��ų� ����Ǿ����ϴ�.";
				ui.textBrowser_Trans->setPlainText(QString::fromLocal8Bit(error.c_str()));
			}
		}
	}
}
 
// ���� ���� â���� ���콺�� ���� ����
void mouseHandler(int event, int x, int y, int flags, void *param)
{
	if (event == CV_EVENT_LBUTTONDOWN && !STT::drag)
	{
		/* left button clicked. STT selection begins */
		STT::point1 = Point(x, y);
		STT::drag = 1;
	}

	if (event == CV_EVENT_MOUSEMOVE && STT::drag)
	{
		/* mouse dragged. STT being selected */
		Mat img1 = STT::img.clone();
		STT::point2 = Point(x, y);
		rectangle(img1, STT::point1, STT::point2, CV_RGB(255, 0, 0), 3, 8, 0);
		imshow("���� ����", img1);
	}

	if (event == CV_EVENT_LBUTTONUP && STT::drag)
	{
		STT::point2 = Point(x, y);

		if (STT::point1.x < STT::point2.x)
			STT::rect = Rect(STT::point1.x, STT::point1.y, STT::point2.x - STT::point1.x, STT::point2.y - STT::point1.y);
		else
			STT::rect = Rect(STT::point2.x, STT::point2.y, STT::point1.x - STT::point2.x, STT::point1.y - STT::point2.y);
		STT::drag = 0;
	}

	if (event == CV_EVENT_LBUTTONUP)
	{
		/* STT selected */
		STT::select_flag = 1;
		STT::drag = 0;
	}
}