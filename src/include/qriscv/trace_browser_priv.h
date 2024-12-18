// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TRACE_BROWSER_PRIV_H
#define QRISCV_TRACE_BROWSER_PRIV_H

#include <sigc++/sigc++.h>
#include <vector>

#include <QPlainTextEdit>

#include "base/trackable_mixin.h"
#include "qriscv/memory_view_delegate.h"
#include "qriscv/stoppoint_list_model.h"
#include "uriscv/types.h"

class Stoppoint;
class StoppointSet;
class Processor;

class TracepointListModel : public BaseStoppointListModel,
                            public TrackableMixin {
  Q_OBJECT

public:
  TracepointListModel(StoppointSet *tracepoints, QObject *parent = 0);

  int columnCount(const QModelIndex &) const { return 1; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;

  bool IsDirty(int row) const { return dirtySet[row]; }
  void ClearDirty(int row);

protected:
  virtual void StoppointAdded();
  virtual void StoppointRemoved(int index);

private:
  void onHit(size_t spIndex, const Stoppoint *stoppoint, Word addr,
             const Processor *cpu);

  // For the uninitiated: beware of the vector<bool> specialization!
  // It might be efficient but has weird semantics. Just ask the
  // mighty Internet.
  typedef std::vector<bool> BitSet;
  BitSet dirtySet;

// We keep this here because of pure desperation: in a perfect
// world disconnections should work automagically via
// sigc::trackable. It refuses to work, however, for some reason
// or other. Sigh.
#if 0
	sigc::connection onHitConnection;
#endif
};

class AsciiView : public QPlainTextEdit, public MemoryViewDelegate {
  Q_OBJECT

public:
  static AsciiView *Create(Word start, Word end);
  virtual void Refresh();

private:
  static const unsigned int kUnicodeReplacementChar = 0xFFFD;

  AsciiView(Word start, Word end);

  const Word start;
  const Word end;
};

#endif // QRISCV_TRACE_BROWSER_PRIV_H
