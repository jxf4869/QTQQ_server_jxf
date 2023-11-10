#include "TcpSocket.h"

TcpSocket::TcpSocket()
{}

TcpSocket::~TcpSocket()
{}

void TcpSocket::run()
{
	//设置套接子的描述符
	m_descriptor = this->socketDescriptor();

	//套接字有数据来时.执行onReceiveData()槽
	connect(this, SIGNAL(readyRead()),   this, SLOT(onReceiveData()));

	//套接字断开连接时.执行onClientDisconnect()槽
	connect(this, SIGNAL(disconnected()),this, SLOT(onClientDisconnect()));

}

void TcpSocket::onReceiveData() {
	QByteArray buffer = this->readAll();//获取收到的数据
	if (!buffer.isEmpty()) {//获取收到的数据不为空
		QString strData = QString::fromLocal8Bit(buffer);
		//发送接受到客户端数据的信号
		emit signalGetDataFromClient(buffer, m_descriptor);
	}
}

void TcpSocket::onClientDisconnect()
{
	//发送接受到客户端数据的信号
	emit signalCilentDisconnect(m_descriptor);
}