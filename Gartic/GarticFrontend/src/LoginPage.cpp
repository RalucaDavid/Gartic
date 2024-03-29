#include "../includes/LoginPage.h"

LoginPage::LoginPage(PageController* controller,QWidget* parent)
{
	loginButton = new QPushButton("Login", this);
	registerButton = new QPushButton("Don't have an account? Signup", this);
	username = new QLineEdit(this);
	password = new QLineEdit(this);
	imageLabel = new QLabel(this);
    layout = new QVBoxLayout(this);
	innerLayout = new QHBoxLayout;
	SetSize();
	StyleElements();
	PlaceElements();
	connect(loginButton, &QPushButton::clicked, controller, [=]() {
		if (controller->VerifyLogin(username->text(), password->text())) 
		{
			controller->ShowPage("MainMenu");
		}
		else 
		{
			QMessageBox::warning(controller, "Login Error", "Incorrect username or password");
		}
	});
	connect(registerButton, &QPushButton::clicked, controller, [controller]() {
		controller->ShowPage("Register");
		});
}

void LoginPage::PlaceElements()
{
	setLayout(layout);
	QPixmap image("resources//Title.png");
	imageLabel->setPixmap(image);
	innerLayout->addWidget(imageLabel);
	innerLayout->setAlignment(Qt::AlignCenter);

	layout->addSpacing(50);
	layout->addLayout(innerLayout);
	layout->addWidget(username);
	layout->addSpacing(5);
	layout->addWidget(password);
	layout->addSpacing(20);
	layout->addWidget(loginButton);
	layout->addSpacing(5);
	layout->addWidget(registerButton);
	layout->addSpacing(200);
	layout->setAlignment(Qt::AlignCenter);
}

void LoginPage::StyleElements()
{
	password->setEchoMode(QLineEdit::Password);
	username->setPlaceholderText("Username");
	password->setPlaceholderText("Password");
	QFile styleFile("resources//style.css");
	styleFile.open(QFile::ReadOnly | QFile::Text);
	QString styleSheet = styleFile.readAll();
	setStyleSheet(styleSheet);
}

void LoginPage::SetSize()
{
	username->setFixedSize(400, 50);
	password->setFixedSize(400, 50);
	loginButton->setFixedSize(400, 50);
	registerButton->setFixedSize(400, 50);

	username->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	password->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	loginButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	registerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

LoginPage::~LoginPage()
{
	/*empty*/
}


