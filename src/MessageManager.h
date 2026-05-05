#pragma once

#include <QWidget>
#include <QList>
#include "Message.h"

class MessageManager : public QWidget
{
	Q_OBJECT

public:
	static MessageManager* instance();
	// duration 表示显示多久ms
	void showMessage(Message::Type type, const QString& message, int msgDuration = 3000);
signals:
	void onNewMsgAdd(Message* msg);
private slots:
	void onMessageRequestExit(Message* msg);
	void startExitAnimation();
	void onMessageExitFinished(Message* msg);
private:
	explicit MessageManager(QWidget* parent = nullptr);
	~MessageManager();
	QRect getScreenGeometry() const;
	void relayoutAllMessages();
private:
	QList<Message*> m_messages;			// 主队列：当前屏幕上存在的所有消息
	static MessageManager* m_instance;
	int spacingY = 10;
	int msgHeight = 0;
	bool isAnimating = false;
	int animDuration = 350;				// 动画组的持续时间
	bool m_isBatchAnimating = false;	// 批量上移是否进行中
};
