#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include "csqlite.h"
#include "customwidget.h"
#include <QCloseEvent>

#define MAX_MUSIC_COUNT 200  //歌单最大容量
#define MAX_LYRIC_COUNT 1000 //歌词最大容量
namespace Ui {
class MusicWidget;
}

class MusicWidget : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit MusicWidget(QWidget *parent = 0);
    ~MusicWidget();

private slots:
    void on_pb_play_clicked();

    void on_pb_addMusic_clicked();

    void on_lw_musicList_doubleClicked(const QModelIndex &index);

    void on_pb_prev_clicked();

    void on_pb_next_clicked();

    //自定义槽函数
    void slot_musicPositionChanged(qint64 val); //  音乐进度条变换
    void on_slider_musicProcess_valueChanged(int value);
    void slot_musicBackgroundImage();
    void loadSqlAndSetMusicList();              //  加载数据库并设置歌单、音量

    void on_pb_min_clicked();

    void on_pb_close_clicked();

    virtual void closeEvent(QCloseEvent *event);//  重写关闭事件

    void on_slider_volume_valueChanged(int value);

    void on_pb_volumn_clicked();

    void on_pb_deleteMusic_clicked();

    void slot_stringMove();

private:
    Ui::MusicWidget *ui;
    QMediaPlayer *m_Player;
    QString m_musicList[MAX_MUSIC_COUNT];       //  缓存歌曲路径
    QString m_musicLyricList[MAX_LYRIC_COUNT];  //  缓存歌词数组
    quint32 m_musicCount;                       //  歌曲数量
    quint32 m_musicLyricCount;                  //  歌词行数
    bool m_musicStartFlag;                      //  歌曲是否播放标志位
    bool m_musicPositionChangedFlag;            //  音乐进度条改变标志位 --true表示 函数设置改变 --false表示 拖动
    QString m_currentMusicName;                 //  当前歌曲名字

    quint32 m_volumnNow;                        //  当前音量
    quint32 m_volumnLast;                       //  之前音量 用于恢复
    bool    m_voiceOpenFlag;                    //  用于标志是否静音
    CSqlite *m_sql;                             //  数据库

    quint32 m_mvStringPos;                      //  当前滚动的位置
    QString m_mvString;                         //  当前滚动的字符串

};

#endif // MUSICWIDGET_H
