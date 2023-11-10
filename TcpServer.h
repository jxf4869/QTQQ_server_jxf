#pragma once

#include <QTcpServer>

class TcpServer  : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();

public:
	bool run();//监听指定端口


signals:
	//客户端有链接来了的信号
	void signalTcpMsgComes(QByteArray&);

private slots:
	//处理数据
	void SocketDataProcessing(QByteArray& SendData, int descriptor);
	//断开连接
	void SocketDisconnected(int descriptor);

protected:
	//客户端有新的套接字连接
	void incomingConnection(qintptr socketDescriptor);
private:
	int m_port;//端口号
	QList<QTcpSocket*> m_tcpSocketConnectList;//客户端的连接链表
};
