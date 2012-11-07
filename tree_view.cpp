// Andrew Naplavkov

#include <algorithm>
#include <brig/database/oracle/client_version.hpp>
#include <brig/database/sqlite/command_allocator.hpp>
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
#include "connection.h"
#include "dialog_connect.h"
#include "dialog_odbc.h"
#include "dialog_shape.h"
#include "global.h"
#include "layer.h"
#include "layer_geometry.h"
#include "tree_view.h"
#include "utilities.h"

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

  m_connect_postgres_act = new QAction(QIcon(":/res/postgres.png"), "connect to Postgres", this);
  m_connect_postgres_act->setIconVisibleInMenu(true);
  connect(m_connect_postgres_act, SIGNAL(triggered()), this, SLOT(on_connect_postgres()));

  m_open_sqlite_act = new QAction(QIcon(":/res/sqlite.png"), "open SQLite file", this);
  m_open_sqlite_act->setIconVisibleInMenu(true);
  connect(m_open_sqlite_act, SIGNAL(triggered()), this, SLOT(on_open_sqlite()));

  m_new_sqlite_act = new QAction(QIcon(":/res/add_folder.png"), "new SQLite file", this);
  m_new_sqlite_act->setIconVisibleInMenu(true);
  connect(m_new_sqlite_act, SIGNAL(triggered()), this, SLOT(on_new_sqlite()));

  m_copy_shp_act = new QAction(QIcon(":/res/shp.png"), "copy shapefile", this);
  m_copy_shp_act->setIconVisibleInMenu(true);
  connect(m_copy_shp_act, SIGNAL(triggered()), this, SLOT(on_copy_shp()));

  m_copy_rendered_layers_act = new QAction(QIcon(":/res/copy.png"), "copy rendered layer(s)", this);
  m_copy_rendered_layers_act->setIconVisibleInMenu(true);
  connect(m_copy_rendered_layers_act, SIGNAL(triggered()), this, SLOT(on_copy_rendered_layers()));

  m_refresh_act = new QAction(QIcon(":/res/refresh.png"), "refresh", this);
  m_refresh_act->setIconVisibleInMenu(true);
  connect(m_refresh_act, SIGNAL(triggered()), this, SLOT(on_refresh()));

  m_use_in_sql_act = new QAction(QIcon(":/res/sql.png"), "use in SQL", this);
  m_use_in_sql_act->setIconVisibleInMenu(true);
  connect(m_use_in_sql_act, SIGNAL(triggered()), this, SLOT(on_use_in_sql()));

  m_paste_layers_act = new QAction(QIcon(":/res/paste.png"), "paste layer(s)", this);
  m_paste_layers_act->setIconVisibleInMenu(true);
  connect(m_paste_layers_act, SIGNAL(triggered()), this, SLOT(on_paste_layers()));

  m_disconnect_act = new QAction(QIcon(":/res/disconnect.png"), "disconnect", this);
  m_disconnect_act->setIconVisibleInMenu(true);
  connect(m_disconnect_act, SIGNAL(triggered()), this, SLOT(on_disconnect()));

  m_zoom_to_fit_act = new QAction(QIcon(":/res/zoom.png"), "zoom to fit", this);
  m_zoom_to_fit_act->setIconVisibleInMenu(true);
  connect(m_zoom_to_fit_act, SIGNAL(triggered()), this, SLOT(on_zoom_to_fit()));

  m_use_projection_act = new QAction(QIcon(":/res/map.png"), "use the projection", this);
  m_use_projection_act->setIconVisibleInMenu(true);
  connect(m_use_projection_act, SIGNAL(triggered()), this, SLOT(on_use_projection()));

  m_attributes_act = new QAction(QIcon(":/res/sql.png"), "attributes", this);
  m_attributes_act->setIconVisibleInMenu(true);
  connect(m_attributes_act, SIGNAL(triggered()), this, SLOT(emit_attributes()));

  m_copy_act = new QAction(QIcon(":/res/copy.png"), "copy", this);
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

  qRegisterMetaType<connection_link>("connection_link");
  qRegisterMetaType<brig::proj::epsg>("brig::proj::epsg");
  qRegisterMetaType<std::shared_ptr<task>>("std::shared_ptr<task>");
  qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
  qRegisterMetaType<std::vector<layer_link>>("std::vector<layer_link>");
  connect
    ( &m_mdl, SIGNAL(signal_commands(connection_link, std::vector<std::string>))
    , this, SLOT(emit_commands(connection_link, std::vector<std::string>))
    );
  connect(&m_mdl, SIGNAL(signal_disconnect(connection_link)), this, SLOT(emit_disconnect(connection_link)));
  connect(&m_mdl, SIGNAL(signal_layers(std::vector<layer_link>)), this, SLOT(emit_layers(std::vector<layer_link>)));
  connect(&m_mdl, SIGNAL(signal_proj(brig::proj::epsg)), this, SLOT(emit_proj(brig::proj::epsg)));
  connect(&m_mdl, SIGNAL(signal_task(std::shared_ptr<task>)), this, SLOT(emit_task(std::shared_ptr<task>)));
  connect(&m_mdl, SIGNAL(signal_view(QRectF, brig::proj::epsg)), this, SLOT(emit_view(QRectF, brig::proj::epsg)));
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

void tree_view::on_open_sqlite()
{
  try
  {
    QSettings settings(SettingsIni, QSettings::IniFormat);
    QFileDialog dlg
      ( this
      , "open SQLite files"
      , settings.value(QString("%1/%2").arg(SettingsSQLite).arg(SettingsPath), QDir::currentPath()).toString()
      , "SQLite files (*.sqlite);;All files (*.*)"
      );
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (dlg.exec() != QDialog::Accepted) return;
    settings.setValue(QString("%1/%2").arg(SettingsSQLite).arg(SettingsPath), dlg.directory().absolutePath());
    wait_cursor w;
    QStringList files = dlg.selectedFiles();
    for (int i(0); i < files.size(); ++i)
      m_mdl.connect_sqlite(files[i], false);
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_new_sqlite()
{
  try
  {
    QSettings settings(SettingsIni, QSettings::IniFormat);
    QFileDialog dlg
      ( this
      , "new SQLite file"
      , settings.value(QString("%1/%2").arg(SettingsSQLite).arg(SettingsPath), QDir::currentPath()).toString()
      , "SQLite files (*.sqlite)"
      );
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (!dlg.exec()) return;
    settings.setValue(QString("%1/%2").arg(SettingsSQLite).arg(SettingsPath), dlg.directory().absolutePath());
    wait_cursor w;
    QFileInfo file(dlg.selectedFiles().value(0));
    if (file.exists() && !QFile::remove(file.filePath()))
    {
      show_message("file removing error");
      return;
    }
    if (file.suffix().isEmpty()) file = QFileInfo(file.dir(), file.fileName() + ".sqlite");
    m_mdl.connect_sqlite(file.filePath(), true);
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_copy_shp()
{
  try
  {
    m_lrs_copy.clear();

    dialog_shape dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    wait_cursor w;
    QFileInfo file(dlg.selectedFiles().value(0));
    std::string base(file.baseName().toUtf8().constData());
    std::string path(QFileInfo(file.dir(), file.baseName()).filePath().toUtf8().constData());
    std::string charset(dlg.charset().toUtf8().constData());
    std::string epsg(dlg.epsg().toUtf8().constData());

    auto allocator(std::make_shared<brig::database::sqlite::command_allocator>(":memory:"));
    connection_link dbc(new connection(allocator, ":memory:"));
    dbc->get_command()->exec("CREATE VIRTUAL TABLE \"" + base + "\" USING VirtualShape('" + path + "', '" + charset + "', " + epsg + ")");

    brig::database::identifier id; id.name = base;
    auto tbl(dbc->get_table_definition(id));
    auto col_geo = tbl["Geometry"];
    if (!col_geo) return;

    col_geo->type = brig::database::Geometry;
    col_geo->dbms_type_lcase.name = "geometry";
    col_geo->srid = dlg.epsg().toInt();
    col_geo->epsg = dlg.epsg().toInt();
    col_geo->query_expression = "AsBinary(\"GEOMETRY\")";

    m_lrs_copy.push_back( layer_link(new layer_geometry(dbc, id, tbl)) );
  }
  catch (const std::exception& e)  { show_message(e.what()); }
}

void tree_view::on_copy_rendered_layers()
{
  m_lrs_copy.clear();
  m_mdl.push_back_rendered_layers(m_lrs_copy);
}

void tree_view::on_copy()
{
  m_lrs_copy.clear();
  if (m_mdl.is_layer(m_idx_menu)) m_lrs_copy.push_back( m_mdl.get_layer(m_idx_menu) );
}

void tree_view::on_update()
{
  m_drop_act->setEnabled(m_mdl.is_layer(m_idx_menu) && m_mdl.get_layer(m_idx_menu)->is_writable());
  m_paste_rows_act->setEnabled(m_drop_act->isEnabled() && (m_lrs_copy.size() == 1) && m_mdl.is_layer(m_idx_menu) && m_mdl.get_layer(m_idx_menu)->get_levels() == m_lrs_copy.front()->get_levels());
  m_paste_layers_act->setEnabled(m_mdl.is_connection(m_idx_menu) && !m_lrs_copy.empty());
  m_copy_rendered_layers_act->setEnabled(m_mdl.has_rendered_layers());
}

void tree_view::on_show_menu(QPoint point)
{
  m_idx_menu = indexAt(point);
  on_update();

  QList<QAction*> actions;
  if (m_mdl.is_connection(m_idx_menu))
  {
    actions.append(m_refresh_act);
    actions.append(m_use_in_sql_act);
    actions.append(m_paste_layers_act);
    actions.append(m_disconnect_act);
    actions.append(m_separator1_act);
  }
  else if (m_mdl.is_layer(m_idx_menu))
  {
    actions.append(m_zoom_to_fit_act);
    actions.append(m_use_projection_act);
    actions.append(m_attributes_act);
    actions.append(m_copy_act);
    actions.append(m_paste_rows_act);
    actions.append(m_drop_act);
    actions.append(m_separator1_act);
  }
  actions.append(m_connect_mysql_act);
  actions.append(m_connect_odbc_act);
  if (!brig::database::oracle::client_version().empty()) actions.append(m_connect_oracle_act);
  actions.append(m_connect_postgres_act);
  actions.append(m_open_sqlite_act);
  actions.append(m_new_sqlite_act);
  actions.append(m_separator2_act);
  actions.append(m_copy_shp_act);
  actions.append(m_copy_rendered_layers_act);
  QMenu::exec(actions, mapToGlobal(point));
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
