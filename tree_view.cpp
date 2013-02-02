// Andrew Naplavkov

#include <algorithm>
#include <brig/database/oracle/client_version.hpp>
#include <brig/proj/shared_pj.hpp>
#include <exception>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QList>
#include <QMenu>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <utility>
#include "dialog_connect.h"
#include "dialog_odbc.h"
#include "global.h"
#include "layer.h"
#include "layer_geometry.h"
#include "provider.h"
#include "task_attributes.h"
#include "tree_view.h"
#include "utilities.h"

enum Provider {
  SQLite = 0,
  OGR,
  GDAL
};

struct file_open_def {
  Provider pvd;
  bool writable;
  const char* filter;
  const char* code;
  const char* ext;
};

static const file_open_def s_filters[] = {
{ OGR, false, "Arc/Info ASCII Coverage (*.e00)", "", "" },
{ OGR, true, "ESRI Shapefiles (*.shp)", "ESRI Shapefile", "shp" },
{ GDAL, false, "GeoTIFF (*.tif *.tiff)", "", "" },
{ OGR, false, "Mapinfo (*.mif *.tab)", "", "" }, // todo:
{ OGR, false, "S-57 Base file (*.000)", "", "" },
{ SQLite, true, "SQLite (*.sqlite)", "", "sqlite" },
};

tree_view::tree_view(QWidget* parent) : QTreeView(parent)
{
  setModel(&m_mdl);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setHeaderHidden(true);
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_show_menu(QPoint)));

  m_connect_mysql_act = new QAction(QIcon(":/res/mysql.png"), "connect to MySQL", this);
  m_connect_mysql_act->setIconVisibleInMenu(true);
  connect(m_connect_mysql_act, SIGNAL(triggered()), this, SLOT(on_connect_mysql()));

  m_connect_odbc_act = new QAction(QIcon(":/res/odbc.png"), "connect using ODBC", this);
  m_connect_odbc_act->setIconVisibleInMenu(true);
  connect(m_connect_odbc_act, SIGNAL(triggered()), this, SLOT(on_connect_odbc()));

  m_connect_oracle_act = new QAction(QIcon(":/res/oracle.png"), "connect to Oracle", this);
  m_connect_oracle_act->setIconVisibleInMenu(true);
  connect(m_connect_oracle_act, SIGNAL(triggered()), this, SLOT(on_connect_oracle()));

  m_connect_osm_act = new QAction(QIcon(":/res/osm.png"), "connect to OSM", this);
  m_connect_osm_act->setIconVisibleInMenu(true);
  connect(m_connect_osm_act, SIGNAL(triggered()), this, SLOT(on_connect_osm()));

  m_connect_postgres_act = new QAction(QIcon(":/res/postgres.png"), "connect to Postgres", this);
  m_connect_postgres_act->setIconVisibleInMenu(true);
  connect(m_connect_postgres_act, SIGNAL(triggered()), this, SLOT(on_connect_postgres()));

  m_open_file_act = new QAction(QIcon(":/res/open.png"), "open file", this);
  m_open_file_act->setIconVisibleInMenu(true);
  connect(m_open_file_act, SIGNAL(triggered()), this, SLOT(on_open_file()));

  m_new_file_act = new QAction(QIcon(":/res/new_file.png"), "new file", this);
  m_new_file_act->setIconVisibleInMenu(true);
  connect(m_new_file_act, SIGNAL(triggered()), this, SLOT(on_new_file()));

  m_copy_checked_act = new QAction(QIcon(":/res/copy.png"), "copy checked layer(s)", this);
  m_copy_checked_act->setIconVisibleInMenu(true);
  connect(m_copy_checked_act, SIGNAL(triggered()), this, SLOT(on_copy_checked()));

  m_refresh_act = new QAction(QIcon(":/res/refresh.png"), "refresh", this);
  m_refresh_act->setIconVisibleInMenu(true);
  connect(m_refresh_act, SIGNAL(triggered()), this, SLOT(on_refresh()));

  m_sql_console_act = new QAction(QIcon(":/res/sql.png"), "SQL console", this);
  m_sql_console_act->setIconVisibleInMenu(true);
  connect(m_sql_console_act, SIGNAL(triggered()), this, SLOT(on_sql_console()));

  m_paste_layers_act = new QAction(QIcon(":/res/paste.png"), "paste layer(s)", this);
  m_paste_layers_act->setIconVisibleInMenu(true);
  connect(m_paste_layers_act, SIGNAL(triggered()), this, SLOT(on_paste_layers()));

  m_disconnect_act = new QAction(QIcon(":/res/disconnect.png"), "disconnect", this);
  m_disconnect_act->setIconVisibleInMenu(true);
  connect(m_disconnect_act, SIGNAL(triggered()), this, SLOT(on_disconnect()));

  m_zoom_to_fit_act = new QAction(QIcon(":/res/zoom.png"), "zoom to fit", this);
  m_zoom_to_fit_act->setIconVisibleInMenu(true);
  connect(m_zoom_to_fit_act, SIGNAL(triggered()), this, SLOT(on_zoom_to_fit()));

  m_use_projection_act = new QAction(QIcon(":/res/map.png"), "use this projection", this);
  m_use_projection_act->setIconVisibleInMenu(true);
  connect(m_use_projection_act, SIGNAL(triggered()), this, SLOT(on_use_projection()));

  m_snap_to_pixels_act = new QAction(QIcon(":/res/pixels.png"), "snap to pixels", this);
  m_snap_to_pixels_act->setIconVisibleInMenu(true);
  connect(m_snap_to_pixels_act, SIGNAL(triggered()), this, SLOT(on_snap_to_pixels()));

  m_attributes_act = new QAction(QIcon(":/res/sql.png"), "attributes", this);
  m_attributes_act->setIconVisibleInMenu(true);
  connect(m_attributes_act, SIGNAL(triggered()), this, SLOT(on_attributes()));

  m_copy_act = new QAction(QIcon(":/res/copy.png"), "copy this layer", this);
  m_copy_act->setIconVisibleInMenu(true);
  connect(m_copy_act, SIGNAL(triggered()), this, SLOT(on_copy()));

  m_paste_rows_act = new QAction(QIcon(":/res/paste.png"), "paste rows", this);
  m_paste_rows_act->setIconVisibleInMenu(true);
  connect(m_paste_rows_act, SIGNAL(triggered()), this, SLOT(on_paste_rows()));

  m_drop_act = new QAction(QIcon(":/res/delete.png"), "drop", this);
  m_drop_act->setIconVisibleInMenu(true);
  connect(m_drop_act, SIGNAL(triggered()), this, SLOT(on_drop()));

  m_separator1_act = new QAction(QIcon(""), "", this);
  m_separator1_act->setSeparator(true);
  m_separator2_act = new QAction(QIcon(""), "", this);
  m_separator2_act->setSeparator(true);

  qRegisterMetaType<provider_ptr>("provider_ptr");
  qRegisterMetaType<brig::proj::shared_pj>("brig::proj::shared_pj");
  qRegisterMetaType<std::shared_ptr<task>>("std::shared_ptr<task>");
  qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
  qRegisterMetaType<std::vector<layer_ptr>>("std::vector<layer_ptr>");
  connect
    ( &m_mdl, SIGNAL(signal_sql(provider_ptr, std::vector<std::string>))
    , this, SLOT(emit_sql(provider_ptr, std::vector<std::string>))
    );
  connect(&m_mdl, SIGNAL(signal_layers(std::vector<layer_ptr>)), this, SLOT(emit_layers(std::vector<layer_ptr>)));
  connect(&m_mdl, SIGNAL(signal_proj(brig::proj::shared_pj)), this, SLOT(emit_proj(brig::proj::shared_pj)));
  connect(&m_mdl, SIGNAL(signal_task(std::shared_ptr<task>)), this, SLOT(emit_task(std::shared_ptr<task>)));
  connect(&m_mdl, SIGNAL(signal_rect(QRectF, brig::proj::shared_pj)), this, SLOT(emit_rect(QRectF, brig::proj::shared_pj)));
  connect(&m_mdl, SIGNAL(signal_scale(double, brig::proj::shared_pj)), this, SLOT(emit_scale(double, brig::proj::shared_pj)));
  connect(&m_mdl, SIGNAL(signal_disconnect(provider_ptr)), this, SLOT(emit_disconnect(provider_ptr)));
}

void tree_view::on_connect_mysql()
{
  try
  {
    dialog_connect dlg(this, QIcon(":/res/mysql.png"), SettingsMySQL, "192.168.1.56", 3306, "test", "root");
    if (dlg.exec() != QDialog::Accepted) return;
    wait_cursor w;
    m_mdl.connect_mysql(dlg.host(), dlg.port(), dlg.db(), dlg.usr(), dlg.pwd());
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_connect_odbc()
{
  try
  {
    dialog_odbc dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    wait_cursor w;
    m_mdl.connect_odbc(dlg.str());
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_connect_oracle()
{
  try
  {
    dialog_connect dlg(this, QIcon(":/res/oracle.png"), SettingsOracle, "192.168.1.152", 1521, "XE", "SYSTEM");
    if (dlg.exec() != QDialog::Accepted) return;
    wait_cursor w;
    m_mdl.connect_oracle(dlg.host(), dlg.port(), dlg.db(), dlg.usr(), dlg.pwd());
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_connect_osm()
{
  m_mdl.connect_osm();
}

void tree_view::on_connect_postgres()
{
  try
  {
    dialog_connect dlg(this, QIcon(":/res/postgres.png"), SettingsPostgres, "gis-lab.info", 5432, "osm_shp", "guest");
    if (dlg.exec() != QDialog::Accepted) return;
    wait_cursor w;
    m_mdl.connect_postgres(dlg.host(), dlg.port(), dlg.db(), dlg.usr(), dlg.pwd());
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_open_file()
{
  using namespace std;
  try
  {
    QSettings settings(SettingsIni, QSettings::IniFormat);
    QStringList filters;
    for (auto iter(begin(s_filters)); iter != end(s_filters); ++iter)
      filters << iter->filter;
    QFileDialog dlg
      ( this
      , "new file"
      , settings.value(QString("%1/%2").arg(SettingsFileOpen).arg(SettingsPath), QDir::currentPath()).toString()
      );
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters(filters);
    dlg.selectNameFilter(settings.value(QString("%1/%2").arg(SettingsFileOpen).arg(SettingsFilter), QString(s_filters[0].filter)).toString());
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (dlg.exec() != QDialog::Accepted) return;

    wait_cursor w;
    const size_t filter_selected(distance(begin(s_filters), find_if
      ( begin(s_filters)
      , end(s_filters)
      , [&dlg](const file_open_def& i){ return dlg.selectedNameFilter().compare(i.filter) == 0; }
      )));
    QStringList files = dlg.selectedFiles();
    for (int i(0); i < files.size(); ++i)
    {
      if (i == 0)
      {
        settings.setValue(QString("%1/%2").arg(SettingsFileOpen).arg(SettingsPath), QFileInfo(files[0]).absolutePath());
        settings.setValue(QString("%1/%2").arg(SettingsFileOpen).arg(SettingsFilter), dlg.selectedNameFilter());
      }
      switch (s_filters[filter_selected].pvd)
      {
      case SQLite: m_mdl.connect_sqlite(files[i]); break;
      case OGR: m_mdl.connect_ogr(files[i]); break;
      case GDAL: m_mdl.connect_gdal(files[i]); break;
      }
    }
  }
  catch (const exception& e)  { show_message(e.what()); }
}

void tree_view::on_new_file()
{
  using namespace std;
  try
  {
    QSettings settings(SettingsIni, QSettings::IniFormat);
    QStringList filters;
    for (auto iter(begin(s_filters)); iter != end(s_filters); ++iter)
      if (iter->writable)
        filters << iter->filter;
    QFileDialog dlg
      ( this
      , "new file"
      , settings.value(QString("%1/%2").arg(SettingsFileNew).arg(SettingsPath), QDir::currentPath()).toString()
      );
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setNameFilters(filters);
    dlg.selectNameFilter(settings.value(QString("%1/%2").arg(SettingsFileNew).arg(SettingsFilter), QString(s_filters[0].filter)).toString());
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (!dlg.exec()) return;

    wait_cursor w;
    QFileInfo info(dlg.selectedFiles()[0]);
    settings.setValue(QString("%1/%2").arg(SettingsFileNew).arg(SettingsPath), info.absolutePath());
    settings.setValue(QString("%1/%2").arg(SettingsFileNew).arg(SettingsFilter), dlg.selectedNameFilter());
    const size_t filter_selected(distance(begin(s_filters), find_if
      ( begin(s_filters)
      , end(s_filters)
      , [&dlg](const file_open_def& i){ return dlg.selectedNameFilter().compare(i.filter) == 0; }
      )));
    if (info.suffix().isEmpty()) info = QFileInfo(info.dir(), QString("%1.%2").arg(info.fileName()).arg(s_filters[filter_selected].ext));
    if (info.exists() && !QFile::remove(info.filePath()))
    {
      show_message("file removing error");
      return;
    }
    switch (s_filters[filter_selected].pvd)
    {
    case SQLite: m_mdl.connect_sqlite(info.absoluteFilePath(), true); break;
    case OGR: m_mdl.connect_ogr(info.absoluteFilePath(), QString(s_filters[filter_selected].code), info.baseName()); break;
    case GDAL: break;
    }
  }
  catch (const exception& e)  { show_message(e.what()); }
}

void tree_view::on_copy_checked()
{
  m_lrs_copy.clear();
  m_mdl.push_back_checked(m_lrs_copy);
}

void tree_view::on_copy()
{
  m_lrs_copy.clear();
  if (m_mdl.is_layer(m_idx_menu)) m_lrs_copy.push_back( m_mdl.get_layer(m_idx_menu) );
}

void tree_view::on_update()
{
  m_drop_act->setEnabled(m_mdl.is_layer(m_idx_menu));
  m_paste_rows_act->setEnabled(m_drop_act->isEnabled() && (m_lrs_copy.size() == 1) && m_mdl.is_layer(m_idx_menu) && m_mdl.get_layer(m_idx_menu)->get_levels() == m_lrs_copy.front()->get_levels());
  m_paste_layers_act->setEnabled(m_mdl.is_provider(m_idx_menu) && !m_lrs_copy.empty());
  m_copy_checked_act->setEnabled(m_mdl.has_checked());
}

void tree_view::on_show_menu(QPoint point)
{
  m_idx_menu = indexAt(point);
  on_update();

  QList<QAction*> actions;
  if (m_mdl.is_provider(m_idx_menu))
  {
    actions.append(m_refresh_act);
    if (m_mdl.get_provider(m_idx_menu)->is_database()) actions.append(m_sql_console_act);
    actions.append(m_paste_layers_act);
    actions.append(m_disconnect_act);
    actions.append(m_separator1_act);
  }
  else if (m_mdl.is_layer(m_idx_menu))
  {
    actions.append(m_zoom_to_fit_act);
    actions.append(m_use_projection_act);
    if (m_mdl.get_layer(m_idx_menu)->is_raster()) actions.append(m_snap_to_pixels_act);
    actions.append(m_attributes_act);
    actions.append(m_copy_act);
    actions.append(m_paste_rows_act);
    actions.append(m_drop_act);
    actions.append(m_separator1_act);
  }
  actions.append(m_connect_mysql_act);
  actions.append(m_connect_odbc_act);
  if (!brig::database::oracle::client_version().empty()) actions.append(m_connect_oracle_act);
  actions.append(m_connect_osm_act);
  actions.append(m_connect_postgres_act);
  actions.append(m_open_file_act);
  actions.append(m_new_file_act);
  actions.append(m_separator2_act);
  actions.append(m_copy_checked_act);
  QMenu::exec(actions, mapToGlobal(point));
}

void tree_view::on_attributes()
{
  qRegisterMetaType<provider_ptr>("provider_ptr");
  qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
  task_attributes* tsk(new task_attributes(m_mdl.get_layer(m_idx_menu)));
  connect
    ( tsk, SIGNAL(signal_sql(provider_ptr, std::vector<std::string>))
    , this, SLOT(emit_sql(provider_ptr, std::vector<std::string>))
    );
  emit signal_task(std::shared_ptr<task>(tsk));
}

void tree_view::on_remove(const QModelIndex& parent, int start, int end, QModelIndex& index)
{
  if (!index.isValid())
    return;
  else if (!parent.isValid())
  {
    const int parent_row(static_cast<tree_item*>(index.internalPointer())->m_parent->position());
    if (parent_row >= start && parent_row <= end) index = QModelIndex();
  }
  else if (index.parent() == parent)
  {
    if (index.row() > end)
      index = index.model()->index(index.row() - (end - start + 1), index.column(), index.parent());
    else if (index.row() >= start)
      index = QModelIndex();
  }
}

void tree_view::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  on_remove(parent, start, end, m_idx_menu);
  on_update();
  QTreeView::rowsAboutToBeRemoved(parent, start, end);
}
