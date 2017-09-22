/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEW_HPP
#define PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEW_HPP

#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include <QAbstractScrollArea>
#include <QSizeF>
#include <QSplitter>

#include <pv/data/signaldata.hpp>
#include <pv/util.hpp>
#include <pv/views/viewbase.hpp>

#include "cursorpair.hpp"
#include "flag.hpp"
#include "tracetreeitemowner.hpp"

using std::list;
using std::unordered_map;
using std::unordered_set;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sigrok {
class ChannelGroup;
}

namespace pv {

class Session;

namespace data {
class Logic;
}

namespace views {

namespace trace {

class CursorHeader;
class DecodeTrace;
class Header;
class Ruler;
class Signal;
class Trace;
class Viewport;
class TriggerMarker;

class CustomScrollArea : public QAbstractScrollArea
{
	Q_OBJECT

public:
	CustomScrollArea(QWidget *parent = nullptr);
	bool viewportEvent(QEvent *event);
};

class View : public ViewBase, public TraceTreeItemOwner
{
	Q_OBJECT

private:
	enum StickyEvents {
		TraceTreeItemHExtentsChanged = 1,
		TraceTreeItemVExtentsChanged = 2
	};

private:
	static const pv::util::Timestamp MaxScale;
	static const pv::util::Timestamp MinScale;

	static const int MaxScrollValue;

	static const int ScaleUnits[3];

public:
	explicit View(Session &session, bool is_main_view=false, QWidget *parent = nullptr);

	Session& session();
	const Session& session() const;

	/**
	 * Returns the signals contained in this view.
	 */
	unordered_set< shared_ptr<Signal> > signals() const;

	virtual void clear_signals();

	void add_signal(const shared_ptr<Signal> signal);

#ifdef ENABLE_DECODE
	virtual void clear_decode_signals();

	virtual void add_decode_signal(shared_ptr<data::DecodeSignal> signal);

	virtual void remove_decode_signal(shared_ptr<data::DecodeSignal> signal);
#endif

	/**
	 * Returns the view of the owner.
	 */
	virtual View* view();

	/**
	 * Returns the view of the owner.
	 */
	virtual const View* view() const;

	Viewport* viewport();

	const Viewport* viewport() const;

	virtual void save_settings(QSettings &settings) const;

	virtual void restore_settings(QSettings &settings);

	/**
	 * Gets a list of time markers.
	 */
	vector< shared_ptr<TimeItem> > time_items() const;

	/**
	 * Returns the view time scale in seconds per pixel.
	 */
	double scale() const;

	/**
	 * Returns the time offset of the left edge of the view in
	 * seconds.
	 */
	const pv::util::Timestamp& offset() const;

	/**
	 * Returns the vertical scroll offset.
	 */
	int owner_visual_v_offset() const;

	/**
	 * Sets the visual v-offset.
	 */
	void set_v_offset(int offset);

	/**
	 * Returns the SI prefix to apply to the graticule time markings.
	 */
	pv::util::SIPrefix tick_prefix() const;

	/**
	 * Returns the number of fractional digits shown for the time markings.
	 */
	unsigned int tick_precision() const;

	/**
	 * Returns period of the graticule time markings.
	 */
	const pv::util::Timestamp& tick_period() const;

	/**
	 * Returns the unit of time currently used.
	 */
	util::TimeUnit time_unit() const;

	/**
	 * Returns the number of nested parents that this row item owner has.
	 */
	unsigned int depth() const;

	void zoom(double steps);
	void zoom(double steps, int offset);

	void zoom_fit(bool gui_state);

	void zoom_one_to_one();

	/**
	 * Sets the scale and offset.
	 * @param scale The new view scale in seconds per pixel.
	 * @param offset The view time offset in seconds.
	 */
	void set_scale_offset(double scale, const pv::util::Timestamp& offset);

	set< shared_ptr<pv::data::SignalData> > get_visible_data() const;

	pair<pv::util::Timestamp, pv::util::Timestamp> get_time_extents() const;

	/**
	 * Enables or disables coloured trace backgrounds. If they're not
	 * coloured then they will use alternating colors.
	 */
	void enable_coloured_bg(bool state);

	/**
	 * Returns true if the trace background should be drawn with a coloured background.
	 */
	bool coloured_bg() const;

	/**
	 * Enable or disable showing sampling points.
	 */
	void enable_show_sampling_points(bool state);

	/**
	 * Enable or disable showing the analog minor grid.
	 */
	void enable_show_analog_minor_grid(bool state);

	/**
	 * Returns true if cursors are displayed. false otherwise.
	 */
	bool cursors_shown() const;

	/**
	 * Shows or hides the cursors.
	 */
	void show_cursors(bool show = true);

	/**
	 * Moves the cursors to a convenient position in the view.
	 */
	void centre_cursors();

	/**
	 * Returns a reference to the pair of cursors.
	 */
	shared_ptr<CursorPair> cursors() const;

	/**
	 * Adds a new flag at a specified time.
	 */
	void add_flag(const pv::util::Timestamp& time);

	/**
	 * Removes a flag from the list.
	 */
	void remove_flag(shared_ptr<Flag> flag);

	/**
	 * Gets the list of flags.
	 */
	vector< shared_ptr<Flag> > flags() const;

	const QPoint& hover_point() const;

	void restack_all_trace_tree_items();

Q_SIGNALS:
	void hover_point_changed(const QPoint &hp);

	void selection_changed();

	/// Emitted when the offset changed.
	void offset_changed();

	/// Emitted when the scale changed.
	void scale_changed();

	void sticky_scrolling_changed(bool state);

	void always_zoom_to_fit_changed(bool state);

	/// Emitted when the tick_prefix changed.
	void tick_prefix_changed();

	/// Emitted when the tick_precision changed.
	void tick_precision_changed();

	/// Emitted when the tick_period changed.
	void tick_period_changed();

	/// Emitted when the time_unit changed.
	void time_unit_changed();

public Q_SLOTS:
	void trigger_event(util::Timestamp location);

private:
	void get_scroll_layout(double &length, pv::util::Timestamp &offset) const;

	/**
	 * Simultaneously sets the zoom and offset.
	 * @param scale The scale to set the view to in seconds per pixel. This
	 * value is clamped between MinScale and MaxScale.
	 * @param offset The offset of the left edge of the view in seconds.
	 */
	void set_zoom(double scale, int offset);

	/**
	 * Find a tick spacing and number formatting that does not cause
	 * the values to collide.
	 */
	void calculate_tick_spacing();

	void adjust_top_margin();

	void update_scroll();

	void reset_scroll();

	void set_scroll_default();

	void determine_if_header_was_shrunk();

	void resize_header_to_fit();

	void update_layout();

	TraceTreeItemOwner* find_prevalent_trace_group(
		const shared_ptr<sigrok::ChannelGroup> &group,
		const unordered_map<shared_ptr<data::SignalBase>,
			shared_ptr<Signal> > &signal_map);

	static vector< shared_ptr<Trace> >
		extract_new_traces_for_channels(
		const vector< shared_ptr<sigrok::Channel> > &channels,
		const unordered_map<shared_ptr<data::SignalBase>,
			shared_ptr<Signal> > &signal_map,
		set< shared_ptr<Trace> > &add_list);

	void determine_time_unit();

	bool eventFilter(QObject *object, QEvent *event);

	void resizeEvent(QResizeEvent *event);

	void update_hover_point();

public:
	void row_item_appearance_changed(bool label, bool content);
	void time_item_appearance_changed(bool label, bool content);

	void extents_changed(bool horz, bool vert);

private Q_SLOTS:

	void on_signal_name_changed();
	void on_splitter_moved();

	void h_scroll_value_changed(int value);
	void v_scroll_value_changed();

	void signals_changed();
	void capture_state_updated(int state);

	void on_segment_changed(int segment);

	virtual void perform_delayed_view_update();

	void process_sticky_events();

	/**
	 * Sets the 'offset_' member and emits the 'offset_changed'
	 * signal if needed.
	 */
	void set_offset(const pv::util::Timestamp& offset);

	/**
	 * Sets the 'scale_' member and emits the 'scale_changed'
	 * signal if needed.
	 */
	void set_scale(double scale);

	/**
	 * Sets the 'tick_prefix_' member and emits the 'tick_prefix_changed'
	 * signal if needed.
	 */
	void set_tick_prefix(pv::util::SIPrefix tick_prefix);

	/**
	 * Sets the 'tick_precision_' member and emits the 'tick_precision_changed'
	 * signal if needed.
	 */
	void set_tick_precision(unsigned tick_precision);

	/**
	 * Sets the 'tick_period_' member and emits the 'tick_period_changed'
	 * signal if needed.
	 */
	void set_tick_period(const pv::util::Timestamp& tick_period);

	/**
	 * Sets the 'time_unit' member and emits the 'time_unit_changed'
	 * signal if needed.
	 */
	void set_time_unit(pv::util::TimeUnit time_unit);

private:
	CustomScrollArea *scrollarea_;
	Viewport *viewport_;
	Ruler *ruler_;
	Header *header_;
	QSplitter *splitter_;

	unordered_set< shared_ptr<Signal> > signals_;

#ifdef ENABLE_DECODE
	vector< shared_ptr<DecodeTrace> > decode_traces_;
#endif

	/// The ID of the currently displayed segment
	int current_segment_;

	/// The view time scale in seconds per pixel.
	double scale_;

	/// The view time offset in seconds.
	pv::util::Timestamp offset_;

	bool updating_scroll_;
	bool settings_restored_;
	bool header_was_shrunk_;

	bool sticky_scrolling_;
	bool coloured_bg_;
	bool always_zoom_to_fit_;

	pv::util::Timestamp tick_period_;
	pv::util::SIPrefix tick_prefix_;
	unsigned int tick_precision_;
	util::TimeUnit time_unit_;

	bool show_cursors_;
	shared_ptr<CursorPair> cursors_;

	list< shared_ptr<Flag> > flags_;
	char next_flag_text_;

	vector< shared_ptr<TriggerMarker> > trigger_markers_;

	QPoint hover_point_;

	unsigned int sticky_events_;
	QTimer lazy_event_handler_;

	// This is true when the defaults couldn't be set due to insufficient info
	bool scroll_needs_defaults_;

	// A nonzero value indicates the v offset to restore. See View::resizeEvent()
	int saved_v_offset_;

	// These are used to determine whether the view was altered after acq started
	double scale_at_acq_start_;
	pv::util::Timestamp offset_at_acq_start_;

	// Used to suppress performing a "zoom to fit" when the session stops. This
	// is needed when the view's settings are restored before acquisition ends.
	// In that case we want to keep the restored settings, not have a "zoom to fit"
	// mess them up.
	bool suppress_zoom_to_fit_after_acq_;
};

} // namespace trace
} // namespace views
} // namespace pv

#endif // PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEW_HPP
