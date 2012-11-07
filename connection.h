// Andrew Naplavkov

#ifndef CONNECTION_H
#define CONNECTION_H

#include <brig/boost/geometry.hpp>
#include <brig/database/connection.hpp>
#include <map>
#include <memory>
#include <QMutex>
#include <QString>

class connection : public brig::database::connection<true>
{
  struct compare {
    bool operator()(const brig::database::identifier& a, const brig::database::identifier& b) const
      { return a.schema < b.schema || (a.schema == b.schema && a.name < b.name); }
  }; // compare

  const QString m_str;
  QMutex m_mutex;
  std::map<brig::database::identifier, brig::database::table_definition, compare> m_tables;

public:
  connection(std::shared_ptr<brig::database::command_allocator> allocator, QString str);
  QString get_string() const  { return m_str; }
  QString get_icon();

  brig::database::table_definition get_table_definition(const brig::database::identifier& tbl);
  void reset_table_definition(const brig::database::identifier& tbl);
  bool try_table_definition(const brig::database::identifier& tbl, brig::database::table_definition& def);

  void set_mbr(const brig::database::identifier& col, const brig::boost::box& box);
}; // connection

#endif // CONNECTION_H
