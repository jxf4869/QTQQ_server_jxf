#pragma once
#include "TcpServer.h"
#include <QtWidgets/QDialog>
#include "ui_QTQQ_Server.h"
#include <qsqlquerymodel.h>
#include <QTimer>
#include <qudpsocket.h>
class QTQQ_Server : public QDialog
{
    Q_OBJECT

public:
    QTQQ_Server(QWidget *parent = nullptr);
    ~QTQQ_Server();

private:
    //初始化组合框的数据
    void initComboBoxData();
    //初始化Tcp
    void initTcpSocket();
    //初始化Udp
    void initUdpSocket();
    //连接数据库
    bool connectMysql();
    //更新的表格数据(参数:部门号,员工号
    void updateTabelData(int DepId = 0, int employeeID = 0);
    //获取公司群QQ号
    int getCompDepID();

    //设置状态
    void setStatusMap();
    //设置部门名称
    void setDepNameMap();
    //设置在线
    void setOnlineMap();

private slots:
    //Udp广播信息
    void onUDPbroadMsg(QByteArray& btDate);
    //刷新表格数据
    void onRefresh();

    //-----点击与槽函数自动连接-----

    //1)根据群查找员工
    void on_queryDepartmentBtn_clicked();
    //2)根据员工QQ号筛选
    void on_queryIDBtn_clicked();
    //3)根据员工QQ号注销员工
    void on_LogoutBtn_clicked();
    //4)添加员工按钮选择图片
    void on_selectPictureBtn_clicked();
    //5)添加员工按钮响应的的槽
    void on_addBtn_clicked();

private:
    Ui::QTQQ_Server ui;
    //公司群QQ号

    int m_compDepID;    //公司群QQ号
    int m_depID;        //部门QQ号
    int m_employeeID;   //员工QQ号


    QTimer* m_timer;//用于定时刷新数据
    QMap<QString, QString>m_statusMap;//状态
    QMap<QString, QString>m_depNameMap;//部门名称
    QMap<QString, QString>m_onlineMap;//在线

    QSqlQueryModel m_queryInfoModel;

    TcpServer* m_tcpServer;//tcp服务端
    QUdpSocket* m_udpsender;//udp广播

    QString m_pixPath;//头像路径

};
