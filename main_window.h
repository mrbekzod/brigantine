// Andrew Naplavkov

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <brig/proj/shared_pj.hpp>
#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QString>
#include <QTabWidget>
#include <QTime>

class main_window : public QMainWindow {
  Q_OBJECT

  QTabWidget* m_tab;
  int m_map_tab, m_sql_tab;
  QLabel *m_pos_stat, *m_map_stat, *m_sql_stat;
  QString m_map_msg, m_sql_msg;
  QTime m_map_time, m_sql_time;
  QAction *m_copy_map_stat, *m_copy_sql_stat;

private slots:
  void on_map_coords(QString msg);
  void on_map_start();
  void on_map_process(QString msg)  { m_map_msg = msg; }
  void on_map_idle();
  void on_map_scene(brig::proj::shared_pj pj);
  void on_sql_start();
  void on_sql_process(QString msg)  { m_sql_msg = msg; }
  void on_sql_idle();
  void on_sql_commands()  { m_tab->setCurrentIndex(m_sql_tab); }
  void on_show_stat_menu(QPoint);
  void on_copy_map_stat();
  void on_copy_sql_stat();

protected:
  virtual void keyPressEvent(QKeyEvent* event);
  virtual void timerEvent(QTimerEvent* event);

public:
  main_window();
}; // main_window

#endif // MAIN_WINDOW_H
