#include "musicwidget.h"
#include "ui_musicwidget.h"
#include <QFileDialog>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

MusicWidget::MusicWidget(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MusicWidget)
{
    ui->setupUi(this);
    m_Player = new QMediaPlayer;

    m_musicCount = 0;                       //  歌曲数量
    m_musicLyricCount = 0;                  //  歌词数
    m_musicStartFlag = false;               //  歌曲是否播放标志位
    m_musicPositionChangedFlag = false;

    m_volumnNow = 0;
    m_volumnLast = 0;
    m_voiceOpenFlag = true;

    m_sql = new CSqlite;
    loadSqlAndSetMusicList();
    //加载数据库
    m_Player->setVolume(30);                //  设置音量

    //进度条
    QObject::connect(m_Player,SIGNAL(positionChanged(qint64)),
                     this,SLOT(slot_musicPositionChanged(qint64)) );
    //设置默认窗口背景
    QPixmap pixmap = QPixmap(":/images/start.png");
    pixmap = pixmap.scaled(this->width(),this->height());   //等比例缩放
    QPalette pal(this->palette());                          //QPalette画板 控件的背景
    pal.setBrush(QPalette::Background ,QBrush(pixmap));
    this->setPalette(pal);

    m_mvStringPos = 0;
    m_mvString = ui->lb_stringmv->text();
    //定时器
    QTimer *tm = new QTimer(this);//退出程序自动回收
    connect( tm,SIGNAL(timeout()),this,SLOT(slot_stringMove()) );

    tm->start(1000);
}

MusicWidget::~MusicWidget()
{
    delete ui;
}

//播放/暂停 按钮
void MusicWidget::on_pb_play_clicked()
{
    int index = ui->lw_musicList->currentRow();

    //异常处理，列表没有歌曲时不播放
    if(m_musicCount == 0)
    {
        //回到初始状态    播放暂停、标志位置false、设置图标
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon(QIcon(":/images/bfzn_play.png"));
        return;
    }
    if(index < 0)
    {
        ui->lw_musicList->setCurrentRow(0);
    }
    //正在播放中
    if(m_musicStartFlag)
    {
        //暂停（播放--暂停）
        m_Player->pause();
        //UI切换
        ui->pb_play->setIcon(QIcon(":/images/bfzn_play.png"));
        //切换播放状态
        m_musicStartFlag = false;
    }
    //已经暂停
    else
    {
        //播放（未开始-->播放、暂停-->播放）
        //判断播放状态    播放状态为停止 or 当前列表选中项 与 当前播放音乐名不一致
        if(m_Player->state() == QMediaPlayer::StoppedState
                || m_currentMusicName != ui->lw_musicList->currentItem()->text())
        {
            //设置媒体文件     -----    暂停时不需要设置/列表中歌曲和选中歌曲不一致，需要设置
            m_Player->setMedia(QUrl::fromLocalFile(m_musicList[ui->lw_musicList->currentIndex().row()]));
            //设置背景
            slot_musicBackgroundImage();
        }
        //播放
        m_Player->play();
        //UI切换
        ui->pb_play->setIcon(QIcon(":/images/bfzn_pause.png"));
        //获取当前播放的歌曲名    存储 并 设置控件
        m_currentMusicName = ui->lw_musicList->currentItem()->text();
        ui->lb_currentMusicName->setText(m_currentMusicName);
        //切换播放状态
        m_musicStartFlag = true;
    }
}

//添加歌曲
void MusicWidget::on_pb_addMusic_clicked()
{
    //弹出窗口，选择音乐文件，获取文件的绝对路径 存路径需要字符数组
    QStringList path = QFileDialog::getOpenFileNames(this,"选择歌曲","./music");    //1.父类 2.标题 3.默认路径
    //去重
    bool hasSame = false;
    for(int i=0;i<path.count();++i)
    {
        hasSame = false;
        //遍历所有歌曲
        for(int j=0;j<m_musicCount;++j)
        {
            //如果选中的歌曲名与缓存列表中的歌曲名相同
            if(path[i] == m_musicList[j])
            {
                hasSame = true;
                break;
            }
        }
        //如果遍历结束 歌曲列表中没有与当前加入的同名的歌曲
        if(!hasSame)
        {
            //将歌曲加入到歌曲列表
            m_musicList[m_musicCount] = path[i];
            m_musicCount++;
            //同时将 歌曲名 添加到控件
            QFileInfo info(path[i]);
            //info.baseName();    //  获取到文件名 /music/稻香.mp3 --->baseName:稻香
            ui->lw_musicList->addItem(info.baseName());
            //插入sql
            QString sqlStr =
                    QString("insert into t_musicList (musicName,musicPath) values('%1','%2')")
                    .arg(info.baseName()).arg(path[i]);//格式化字符串
            m_sql->UpdateSql(sqlStr);


        }
    }
//    if(m_musicCount != 0)
//    {
//        ui->lw_musicList->setCurrentRow(0);
//    }

}

//双击列表歌曲播放
void MusicWidget::on_lw_musicList_doubleClicked(const QModelIndex &index)
{
    //获取当前列表选中项的绝对地址-->设置媒体文件
    m_Player->setMedia(QUrl::fromLocalFile(m_musicList[ui->lw_musicList->currentIndex().row()]));
    //播放
    m_Player->play();
    //UI切换
    ui->pb_play->setIcon(QIcon(":/images/bfzn_pause.png"));
    //获取当前播放的歌曲名    存储 并 设置控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText(m_currentMusicName);
    //切换播放状态
    m_musicStartFlag = true;
    //设置背景
    slot_musicBackgroundImage();
}

//上一曲
void MusicWidget::on_pb_prev_clicked()
{
    //异常处理，没有歌曲的情况
    if(m_musicCount == 0)
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon(QIcon(":/images/bfzn_play.png"));
        return;
    }
    //切换到上一曲    第一首歌切换到最后一首
    if(ui->lw_musicList->currentIndex().row() - 1 >= 0)
    {
        //如果当前不是从第一首开始
        //设置媒体文件    获取当前列表选中项的绝对地址
        m_Player->setMedia(QUrl::fromLocalFile(m_musicList[ui->lw_musicList->currentIndex().row() - 1 ]));
        //UI切换  设置当前焦点
        ui->lw_musicList->setCurrentRow(ui->lw_musicList->currentIndex().row() - 1);
    }else
    {
        //当前是从第一首歌开始
        m_Player->setMedia(QUrl::fromLocalFile(m_musicList[m_musicCount -1 ]));
        ui->lw_musicList->setCurrentRow(m_musicCount -1);
    }
    //播放
    m_Player->play();
    //UI切换
    ui->pb_play->setIcon(QIcon(":/images/bfzn_pause.png"));
    //存储当前播放的歌曲名    存缓存 设置控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText(m_currentMusicName);
    //切换播放状态
    m_musicStartFlag = true;
    //设置背景
    slot_musicBackgroundImage();
}

//下一曲
void MusicWidget::on_pb_next_clicked()
{
    //异常处理，没有歌曲的情况
    if(m_musicCount == 0)
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon(QIcon(":/images/bfzn_play.png"));
        return;
    }
    //切换到下一曲    最后一首歌切换到第一首
    //设置媒体文件    获取当前列表选中项的绝对地址
    //循环取余
    m_Player->setMedia(QUrl::fromLocalFile(
                           m_musicList[(ui->lw_musicList->currentIndex().row() + 1) % m_musicCount]
                       ));

    //UI切换  设置当前焦点
    ui->lw_musicList->setCurrentRow((ui->lw_musicList->currentIndex().row() + 1) % m_musicCount);
    //播放
    m_Player->play();
    //UI切换
    ui->pb_play->setIcon(QIcon(":/images/bfzn_pause.png"));
    //获取当前播放的歌曲名    储存 并 设置控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText(m_currentMusicName);
    //切换播放状态
    m_musicStartFlag = true;
    //设置背景
    slot_musicBackgroundImage();
}

//音乐进度条变换
void MusicWidget::slot_musicPositionChanged(qint64 val)
{
    static bool lrcBeginFlag = false;
    if(m_Player->state() == QMediaPlayer::PlayingState)
    {
        if(m_Player->duration())
        {
            //1.计算当前歌曲时间（总时间） 用QTime
            //m_Player->duration() --》歌曲持续时间（单位为：ms）
            //m_Player->position() --》歌曲当前进度（单位为：ms）

            //保存当前播放时间 参数：（小时、分钟、秒、毫秒）       //qRound四舍五入，传参是个小数
            QTime duration1(0,m_Player->position()/60000,           //毫秒数 换算成分钟数
                            qRound(m_Player->position()%60000/1000.0),//计算秒数
                            0);
            //总时间
            QTime duration2(0,m_Player->duration()/60000,           //毫秒数 换算成分钟数
                            qRound(m_Player->duration()%60000/1000.0),//计算秒数
                            0);
            //1.1更新进度条  计算当前时间百分比
            m_musicPositionChangedFlag = true;
            ui->slider_musicProcess->setValue(m_Player->position()*100/m_Player->duration());
            //1.2更新音乐时间
            QString tm1 = duration1.toString("mm:ss");
            QString tm2 = duration2.toString("mm:ss");
            tm1 = tm1 + "/" +tm2;
            ui->lb_currentMusicTime->setText(tm1);
            //2.当前 == 总切歌 -->下一首 播放模式是下一曲
            if(duration1 == duration2)
            {
                //下一曲
                this->on_pb_next_clicked();
            }
            //歌词显示逻辑
            //歌词数组第一个存歌曲名
            if(m_musicLyricList[0] != m_currentMusicName)
            {
                //如果歌曲名不匹配，加载歌词
                //根据歌曲名在路径中加载歌词
                QFile lrcFile("./lrc/"+m_currentMusicName+".txt");
                ui->lw_lyric->clear();//清空之前的歌词
                if(lrcFile.open(QIODevice::ReadOnly))
                {
                    //加载歌词，读每一行到数组
                    QTextStream lrcFileStream(&lrcFile);
                    lrcFileStream.setCodec("UTF-8");
                    lrcBeginFlag = true;
                    m_musicLyricCount = 0;
                    m_musicLyricList[m_musicLyricCount++] = m_currentMusicName;
                    while(!lrcFileStream.atEnd())
                    {
                        //一直读到文件结尾，按行读
                        QString line = lrcFile.readLine();
                        m_musicLyricList[m_musicLyricCount++] = line;
                    }
                    lrcFile.close();
                }else
                {
                    //没有歌词
                    ui->lw_lyric->clear();//清空之前的歌词
                    m_musicLyricCount = 0;
                    ui->lw_lyric->addItem("");
                    ui->lw_lyric->addItem("当前歌曲无歌词");
                    ui->lw_lyric->addItem("");

                    ui->lw_lyric->setCurrentRow(1);
                    //字体变大
                    QFont font;
                    font.setPointSize(18);
                    ui->lw_lyric->currentItem()->setFont(font);
                    ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter); //居中对齐
                    ui->lw_lyric->currentItem()->setTextColor(Qt::red);          //设置颜色

                }
            }else
            {
                //初次加载歌词，避免前几行时间没有匹配，暂时无歌词的情况
                if(lrcBeginFlag)
                {
                    lrcBeginFlag = false;
                    //初次加载歌词前9行
                    int lrcAddLine = 0;
                    for(int i=0;i < m_musicLyricCount && i < 9;++i)
                    {
                        QStringList lrc = m_musicLyricList[i].split(']');//根据]分割字符串
                        if(lrc.count() == 2)
                        {
                            ui->lw_lyric->addItem(lrc.at(1));
                        }else
                        {
                            ui->lw_lyric->addItem(m_musicLyricList[i]);
                        }

                        lrcAddLine++;
                        ui->lw_lyric->setCurrentRow(lrcAddLine-1);
                        ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);//设置居中
                    }
                }
                //已经加载歌词，显示
                //匹配时间
                int currentMusicRow = 0;
                for(currentMusicRow = 0;currentMusicRow < m_musicLyricCount;++currentMusicRow)
                {
                    QString min,sec;
                    //QString的mid函数，取字符串的子串 [00:29.460]我是浪花的泡沫
                    min = m_musicLyricList[currentMusicRow].mid(1,2);
                    sec = m_musicLyricList[currentMusicRow].mid(4,2);
                    QTime duration3(0,min.toInt(),sec.toInt(),0);
                    if(duration1 == duration3)
                    {

                        break;
                    }
                }
                //读取currentMusicRow 上4行、下4行 显示到ui，中间行高亮
                //处理越界
                if(currentMusicRow < m_musicLyricCount)
                {
                    ui->lw_lyric->clear();
                    int lrcAddLineCount = 0;    //用于控件显示的临时变量，不断设置居中
                    for(int i=currentMusicRow-4;i<=currentMusicRow+4;++i)
                    {
                        if(i>=0&&i<m_musicLyricCount)
                        {
                            //不可越界
                            //提取歌词
                            QStringList lrc = m_musicLyricList[i].split(']');//根据]分割字符串
                            if(lrc.count() == 2)
                            {
                                ui->lw_lyric->addItem(lrc.at(1));
                            }else
                            {
                                ui->lw_lyric->addItem(m_musicLyricList[i]);
                            }

                            lrcAddLineCount++;
                            ui->lw_lyric->setCurrentRow(lrcAddLineCount-1);
                            ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);//设置居中
                        }else
                        {
                            //越界补齐(空字符串)
                            ui->lw_lyric->addItem("");
                            lrcAddLineCount++;
                            ui->lw_lyric->setCurrentRow(lrcAddLineCount-1);
                            ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);//设置居中
                        }
                        //高亮
                        if(i == currentMusicRow)
                        {
                            QFont font;
                            font.setPointSize(18);
                            ui->lw_lyric->setCurrentRow(4);                                 //控件上是9行空间
                            ui->lw_lyric->currentItem()->setFont(font);
                            ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter); //居中对齐
                            ui->lw_lyric->currentItem()->setTextColor(Qt::red);             //设置颜色
                        }
                    }
                }
            }
            ui->lw_lyric->setCurrentRow(-1);
        }
    }
}


//进度条 进度改变  //1.手动调整进度条引发 //2.更新进度条引发
void MusicWidget::on_slider_musicProcess_valueChanged(int value)
{
    if(m_musicPositionChangedFlag == false)
    {
        //手动拖动
        if(m_musicStartFlag)
        {
            //如果是播放状态   暂停-设置-播放
            m_Player->pause();
            m_Player->setPosition(value * m_Player->duration() / 100);
            m_Player->play();
        }else
        {
            //如果是暂停状态   设置-播放
            m_Player->setPosition(value * m_Player->duration() / 100);
        }
    }else
    {
        //函数设置
        m_musicPositionChangedFlag = false;
    }
}

//切换音乐背景图片
void MusicWidget::slot_musicBackgroundImage()
{
    //设置默认窗口背景
    QPixmap pixmap = QPixmap("./images/" + m_currentMusicName + ".png");
    if(pixmap.isNull())
    {
        pixmap = QPixmap("./" + m_currentMusicName + ".jpg");
        if(pixmap.isNull())
        {
            pixmap = QPixmap(":/images/start.png");
        }
    }
    pixmap = pixmap.scaled(this->width(),this->height());   //等比例缩放
    QPalette pal(this->palette());                          //QPalette画板 控件的背景
    pal.setBrush(QPalette::Background ,QBrush(pixmap));     //
    this->setPalette(pal);
}

void MusicWidget::loadSqlAndSetMusicList()  //加载sql 并设置歌单、音量
{
    //首先获取路径 设置sql
    QString DBDir = QDir::currentPath() + "/sql/";
    QString fileName = "music.db";
    QDir tempDir;
    tempDir.setPath(DBDir);
    //判断路径是否存在     有路径-加载   没有-创建
    if(!tempDir.exists(DBDir))
    {
        qDebug()<<"不存在该路径";
        tempDir.mkdir(DBDir);
    }

    //image music lrc
    if(!tempDir.exists(QDir::currentPath()+"/music/"))
    {
        qDebug()<<"不存在该路径";
        tempDir.mkdir(QDir::currentPath()+"/music/");
    }
    if(!tempDir.exists(QDir::currentPath()+"/images/"))
    {
        qDebug()<<"不存在该路径";
        tempDir.mkdir(QDir::currentPath()+"/images/");
    }
    if(!tempDir.exists(QDir::currentPath()+"/lrc/"))
    {
        qDebug()<<"不存在该路径";
        tempDir.mkdir(QDir::currentPath()+"/lrc/");
    }
    //判断数据库是否存在   有数据库-加载 没有-创建
    QFile *tempFile = new QFile;
    if(tempFile->exists(DBDir+fileName))
    {
        qDebug()<<"数据库存在 正在加载...";
        m_sql->ConnectSql(DBDir+fileName);  //连接数据库 传入数据库绝对路径
        QStringList resList;
        QString sqlStr = "select musicName ,musicPath from t_musicList;";
        bool res = m_sql->SelectSql(sqlStr,2,resList);
        if(!res)
        {
            return;
        }
        for(int i = 0 ; i < resList.count() ; i += 2)
        {
            ui->lw_musicList->addItem(resList[i]);          //名字
            m_musicList[m_musicCount++] = resList[i+1];     //路径
        }
        //音量
        resList.clear();
        sqlStr = "select volumn from t_volumn;";
        res = m_sql->SelectSql(sqlStr,1,resList);
        if(!res)    return;
        if(!resList.isEmpty())
        {
            m_volumnLast = m_volumnNow = ((QString)resList[0]).toInt();
            ui->slider_volume->setValue(m_volumnNow);
        }

    }else
    {
        qDebug()<<"数据库不存在 正在创建...";
        tempFile->setFileName(DBDir+fileName);
        if(!tempFile->open(QIODevice::WriteOnly | QIODevice::Text))  //创建文件
        {
            qDebug()<<"数据库创建失败！";
            QMessageBox::information(this,"提示","数据库创建失败！无法写入歌曲");
        }else
        {
            qDebug()<<"数据库创建";
            tempFile->close();
            m_sql->ConnectSql(DBDir+fileName);  //连接数据库 传入数据库绝对路径
            //创建表
            QString sqlStr = "create table t_musicList (musicName varchar(260),musicPath varchar(260));";
            m_sql->UpdateSql(sqlStr);

            //创建音量
            sqlStr = "create table t_volumn (volumn int);";
            m_sql->UpdateSql(sqlStr);

            //插入音量大小
            sqlStr = "insert into t_volumn (volumn) values(30);";
            m_sql->UpdateSql(sqlStr);

            //设置音量  所有相关ui联动
            ui->slider_volume->setValue(30);
        }
    }
}

//最小化窗口
void MusicWidget::on_pb_min_clicked()
{
    this->showMinimized();
}

//退出窗口
void MusicWidget::on_pb_close_clicked()
{
    //关闭子窗口
    //关闭主窗口
    this->close();
}
//关闭事件
void MusicWidget::closeEvent(QCloseEvent *event)
{   
    //close窗口 触发关闭事件    QMessageBox::information有警示音
    if(QMessageBox::question(this,"关闭程序","是否关闭程序？") == QMessageBox::Yes)
    {
        QString sqlStr = QString("update t_volumn set volumn = %1").arg(m_volumnNow);
        m_sql->UpdateSql(sqlStr);
        event->accept();    //执行关闭
    }else
    {
        event->ignore();    //忽略
    }

}

//音量改变
void MusicWidget::on_slider_volume_valueChanged(int value)
{
    m_Player->setVolume(value);
    if(m_volumnNow != value)
    {
        m_volumnLast = m_volumnNow;
        m_volumnNow = value;
    }
    //更新控件
    ui->lb_volume->setText(QString::number(value)+"%");
    //是否静音
    if(value == 0)
    {
        if(m_voiceOpenFlag)
        {
            ui->pb_volumn->setIcon(QIcon(":/images/voice_close.png"));

        }
        m_voiceOpenFlag = false;
    }else
    {
        if(!m_voiceOpenFlag)
        {
            ui->pb_volumn->setIcon(QIcon(":/images/voice_open.png"));
        }
        m_voiceOpenFlag = true;
    }


}

//单击切换 静音/非静音
void MusicWidget::on_pb_volumn_clicked()
{
    if(m_voiceOpenFlag)//当前是非静音-->静音
    {
        m_voiceOpenFlag = false;
        ui->pb_volumn->setIcon(QIcon(":/images/voice_close.png"));
        ui->slider_volume->setValue(0);

    }else
    {
        m_voiceOpenFlag = true;
        ui->pb_volumn->setIcon(QIcon(":/images/voice_open.png"));
        ui->slider_volume->setValue(m_volumnLast);
    }
}

//删除歌曲
void MusicWidget::on_pb_deleteMusic_clicked()
{
    //列表中没有歌曲-->无法删除
    if(m_musicCount <= 0)   return;
    //获取要删除的选中项下标
    int index = ui->lw_musicList->currentRow();
    if(index < 0)   return;
    //删除数据库记录
    QString sqlStr = QString("delete from t_musicList where musicPath = '%1'")
            .arg(m_musicList[index]);
    m_sql->UpdateSql(sqlStr);
    //删除数组count--
    for(int i = index ; i < m_musicCount-1; ++i)
    {
        m_musicList[i] = m_musicList[i+1];
    }
    m_musicCount--;
    //删除控件takeitem
    ui->lw_musicList->takeItem(index);
    ui->lw_musicList->setCurrentRow(-1);
    //顺延播放下一曲

}
void MusicWidget::slot_stringMove()
{
    if(m_mvStringPos < m_mvString.length())
    {
        QString tmp = m_mvString.mid(m_mvStringPos) + m_mvString.mid(0,m_mvStringPos);
        ui->lb_stringmv->setText(tmp);
        m_mvStringPos++;
    }else
    {
        m_mvStringPos = 0;
    }
}
