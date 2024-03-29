#include "../includes/GamePage.h"

GamePage::GamePage(PageController* controller, QWidget* parent)
{
	imageLabel = new QLabel(this);
	layout = new QVBoxLayout(this);
	topLayout = new QHBoxLayout();
	listPlayers = new QTextEdit("Players:", this);
	round = new QLabel(this);
	time = new QLabel(this);
	word = new QLabel(this);
	chatLayout = new QVBoxLayout();
	messageInput = new QLineEdit(this);
	sendButton = new QPushButton("Send", this);
	chatHistory = new QTextEdit(this);
	gameLayout = new QHBoxLayout();
	buttonsLayout = new QHBoxLayout();
	board = new BoardWidget();
	gameGridLayout = new QGridLayout();
	drawButton = new QPushButton("Draw", this);
	eraseButton = new QPushButton("Erase", this);
	timer= new QTimer(this);
	timerGame = new QTimer(this);
	currentMode = true;
	isGameStarted = false;
	m_controller = controller;
	previousTime = 0;
	SetSize();
	StyleElements();
	PlaceElements();
	connect(sendButton, &QPushButton::clicked, this, &GamePage::SendMessage);
	connect(board, &BoardWidget::MouseDraw, this, &GamePage::UpdateBoardDraw);
	connect(drawButton, &QPushButton::clicked, this, &GamePage::SetDrawMode);
	connect(eraseButton, &QPushButton::clicked, this, &GamePage::SetEraseMode);
	connect(timer, &QTimer::timeout, this, &GamePage::UpdateDataFromGame);
}

void GamePage::PlaceElements()
{
	setLayout(layout);
	QPixmap image("resources//Title.png");
	imageLabel->setPixmap(image);
	imageLabel->setFixedSize(image.width(), image.height());
	topLayout->addWidget(imageLabel);

	topLayout->setAlignment(imageLabel, Qt::AlignLeft);
	topLayout->addWidget(round);
	topLayout->setAlignment(round, Qt::AlignRight);
	topLayout->addWidget(time);
	topLayout->setAlignment(time, Qt::AlignRight);
	topLayout->addWidget(word);
	topLayout->setAlignment(word, Qt::AlignRight);
	topLayout->addSpacing(100);

	chatLayout->addWidget(chatHistory);
	chatLayout->addWidget(messageInput);
	chatLayout->addWidget(sendButton);

	gameGridLayout->setContentsMargins(20, 0, 20, 0);
	gameGridLayout->addWidget(listPlayers, 0, 0, 1, 1);
	gameGridLayout->addWidget(board, 0, 1, 3, 2);
	gameGridLayout->addWidget(drawButton, 3, 1, 1, 1);
	gameGridLayout->addWidget(eraseButton, 3, 2, 1, 1);
	gameGridLayout->addLayout(chatLayout, 0, 3, 1, 1);

	layout->addLayout(topLayout);
	layout->addLayout(gameGridLayout);
	layout->setAlignment(topLayout, Qt::AlignTop);
	layout->addSpacing(100);
}

void GamePage::StyleElements()
{
	round->setAccessibleName("statusLabel");
	time->setAccessibleName("statusLabel");
	word->setAccessibleName("statusLabel");

	QFile styleFile("resources//style.css");
	styleFile.open(QFile::ReadOnly | QFile::Text);
	QString styleSheet = styleFile.readAll();
	setStyleSheet(styleSheet);
}

void GamePage::SetSize()
{
	chatHistory->setReadOnly(true);
	listPlayers->setReadOnly(true);

	messageInput->setFixedSize(300, 50);
	sendButton->setFixedSize(300, 50);
	listPlayers->setFixedSize(200, 450);
	round->setFixedSize(200, 50);
	time->setFixedSize(200, 50);
	word->setFixedSize(200, 50);
	drawButton->setFixedSize(300, 50);
	eraseButton->setFixedSize(300, 50);
	chatHistory->setFixedSize(300, 350);

	messageInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sendButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	listPlayers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	round->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	time->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	word->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	chatHistory->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	drawButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	eraseButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void GamePage::UpdateBoardDraw(QMouseEvent* event)
{
	if (painter)
	{
		QPoint localPos = event->localPos().toPoint();
		int col = localPos.x() / 10;
		int row = localPos.y() / 10;
		if ((col >= 0 && col < board->GetWidth()) && (row >= 0 && row < board->GetHeight()))
		{
			if (currentMode)
			{
				board->pointsCoordinates.insert({ row, col });
			}
			else
			{
				board->pointsCoordinates.erase({row,col});
			}
			board->update();
		}
	}
}

void GamePage::SetDrawMode()
{
	currentMode = true;
}

void GamePage::SetEraseMode()
{
	currentMode = false;
}

void GamePage::showEvent(QShowEvent* event)
{
	if (!isGameStarted)
	{
		player = m_controller->GetPlayer();
		QMessageBox::StandardButton reply;
		reply = QMessageBox::information(this, "Game", "The game has begun.", QMessageBox::Ok);
		if (reply == QMessageBox::Ok)
		{
			UpdateDataFromGame();
		}
		isGameStarted = true;
	}
	QWidget::showEvent(event);
}

void GamePage::UpdateBoard()
{
	if (painter)
	{
		messageInput->setEnabled(false);
		nlohmann::json boardInput = board->GetBoard();
		if (!boardInput.empty())
		{
			auto responseBoard = cpr::Get(
				cpr::Url{ "http://localhost:18080/sendboard" },
				cpr::Body{ boardInput.dump() }
			);
		}
	}
	else
	{
		auto responseBoard = cpr::Get(
			cpr::Url{ "http://localhost:18080/getboard" }
		);
		if (!responseBoard.text.empty())
		{
			auto boardOutput = nlohmann::json::parse(responseBoard.text);
			std::unordered_set<std::pair<int, int>, HashFunction> newCoordinates;
			std::string x{}, y{};
			for (const auto& jsonEntry : boardOutput)
			{
				if (jsonEntry.find("x") == jsonEntry.end())
					break;
				x = jsonEntry["x"];
				if (jsonEntry.find("y") == jsonEntry.end())
					break;
				y = jsonEntry["y"];
				newCoordinates.insert({ std::stoi(x), std::stoi(y) });
			}
			board->SetBoard(newCoordinates);
		}
	}
}

void GamePage::UpdateTimer()
{
	auto responseTimer = cpr::Get(
		cpr::Url{ "http://localhost:18080/gettimer" });
	int difference = 60 - std::stoul(responseTimer.text);
	if (previousTime<difference)
	{
		messageInput->setEnabled(true);
		board->pointsCoordinates.clear();
	}
	previousTime = difference;
	time->setText("Time:" + QString::number(difference));
}

void GamePage::UpdateRound()
{
	auto responseRound = cpr::Get(
		cpr::Url{ "http://localhost:18080/getroundnumber" });
	round->setText("Round: " + QString::fromUtf8(responseRound.text.c_str()) + "/4");
}

bool GamePage::UpdateStatus()
{
	auto responseStatus = cpr::Get(
		cpr::Url{ "http://localhost:18080/getgamestatus" }
	);
	if (responseStatus.text == "2")
	{
		timer->stop();
		return false;
	}
	return true;
}

void GamePage::UpdatePlayers()
{
	auto responsePlayers = cpr::Get(
		cpr::Url{ "http://localhost:18080/getplayersdatafromgame" });
	auto players = nlohmann::json::parse(responsePlayers.text);
	listPlayers->clear();
	std::string playerInfo, playerScore;
	bool isFirst = true;
	for (const auto& jsonEntry : players)
	{
		if (jsonEntry.find("username") != jsonEntry.end())
		{
			playerInfo = jsonEntry["username"];
		}
		if (jsonEntry.find("score") != jsonEntry.end())
		{
			if (isFirst)
			{
				if (playerInfo == player.GetUsername())
					painter = true;
				else
					painter = false;
				isFirst = false;
			}
			playerScore = jsonEntry["score"];
			playerInfo = playerInfo + "     " + playerScore.substr(0, playerScore.find('.') + 3);
			listPlayers->append(QString::fromUtf8(playerInfo));
		}
	}
}

void GamePage::UpdateChat()
{
	auto responseChat = cpr::Get(
		cpr::Url{ "http://localhost:18080/getchat" },
		cpr::Parameters{
			{ "username", player.GetUsername()},
		});
	auto chat = nlohmann::json::parse(responseChat.text);
	if (!chat.empty())
	{
		chatHistory->clear();
		for (const auto& jsonEntry : chat)
		{
			if (jsonEntry.find("message") != jsonEntry.end())
			{
				std::string messageText = jsonEntry["message"];
				chatHistory->append(QString::fromUtf8(messageText));
			}
		}
	}
}

void GamePage::UpdateWord()
{
	auto responseWord = cpr::Get(
		cpr::Url{ "http://localhost:18080/getword" },
		cpr::Parameters{
					{ "username", player.GetUsername()},
		}
	);
	auto wordJson = nlohmann::json::parse(responseWord.text);
	std::string wordText = wordJson["Word"].get<std::string>();
	std::string spacedWordText;
	for (char character : wordText)
	{
		spacedWordText += character;
		spacedWordText += ' ';
	}
	word->setText("Word: " + QString::fromUtf8(spacedWordText));
}

void GamePage::UpdateDataFromGame()
{
	UpdateBoard();
	UpdateTimer();
	UpdateRound();
	UpdatePlayers();
	UpdateChat();
	UpdateWord();
	if (!UpdateStatus())
	{
		listPlayers->clear();
		round->clear();
		chatHistory->clear();
		time->clear();
		word->clear();
		board->pointsCoordinates.clear();
		isGameStarted = false;
		m_controller->ShowPage("Leaderboard");
		return;
	}
	timer->start(500);
}

void GamePage::SendMessage()
{
	QString message = messageInput->text();
	if (!message.isEmpty())
	{
		std::string messageText = message.toUtf8().constData();
		auto responseChat = cpr::Get(
			cpr::Url{ "http://localhost:18080/addmessagetochat" },
			cpr::Parameters{
				{ "username", player.GetUsername()},
				{"message", messageText},
			});
		messageInput->clear();
		if (responseChat.status_code == 200)
			qDebug() << "The message was sent with success.";
		else if (responseChat.status_code == 201)
		{
			messageInput->setEnabled(false);
		}
		else
			qDebug() << "The message was not sent with success.";
	}
}

GamePage::~GamePage()
{
	/*empty*/
}