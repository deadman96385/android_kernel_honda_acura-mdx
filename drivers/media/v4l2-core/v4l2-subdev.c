/*
 * V4L2 sub-device
 *
 * Copyright (C) 2010 Nokia Corporation
 *
 * Contact: Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *	    Sakari Ailus <sakari.ailus@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <linux/export.h>

#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>

static int subdev_fh_init(struct v4l2_subdev_fh *fh, struct v4l2_subdev *sd)
{
#if defined(CONFIG_VIDEO_V4L2_SUBDEV_API)
	if (sd->entity.num_pads) {
		fh->pad = v4l2_subdev_alloc_pad_config(sd);
		if (fh->pad == NULL)
			return -ENOMEM;
	}
#endif
	return 0;
}

static void subdev_fh_free(struct v4l2_subdev_fh *fh)
{
#if defined(CONFIG_VIDEO_V4L2_SUBDEV_API)
	v4l2_subdev_free_pad_config(fh->pad);
	fh->pad = NULL;
#endif
}

static int subdev_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	struct v4l2_subdev_fh *subdev_fh;
#if defined(CONFIG_MEDIA_CONTROLLER)
	struct media_entity *entity = NULL;
#endif
	int ret;

	subdev_fh = kzalloc(sizeof(*subdev_fh), GFP_KERNEL);
	if (subdev_fh == NULL)
		return -ENOMEM;

	ret = subdev_fh_init(subdev_fh, sd);
	if (ret) {
		kfree(subdev_fh);
		return ret;
	}

	v4l2_fh_init(&subdev_fh->vfh, vdev);
	v4l2_fh_add(&subdev_fh->vfh);
	file->private_data = &subdev_fh->vfh;
#if defined(CONFIG_MEDIA_CONTROLLER)
	if (sd->v4l2_dev->mdev) {
		entity = media_entity_get(&sd->entity);
		if (!entity) {
			ret = -EBUSY;
			goto err;
		}
	}
#endif

	if (sd->internal_ops && sd->internal_ops->open) {
		ret = sd->internal_ops->open(sd, subdev_fh);
		if (ret < 0)
			goto err;
	}

	return 0;

err:
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_put(entity);
#endif
	v4l2_fh_del(&subdev_fh->vfh);
	v4l2_fh_exit(&subdev_fh->vfh);
	subdev_fh_free(subdev_fh);
	kfree(subdev_fh);

	return ret;
}

static int subdev_close(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	struct v4l2_fh *vfh = file->private_data;
	struct v4l2_subdev_fh *subdev_fh = to_v4l2_subdev_fh(vfh);

	if (sd->internal_ops && sd->internal_ops->close)
		sd->internal_ops->close(sd, subdev_fh);
#if defined(CONFIG_MEDIA_CONTROLLER)
	if (sd->v4l2_dev->mdev)
		media_entity_put(&sd->entity);
#endif
	v4l2_fh_del(vfh);
	v4l2_fh_exit(vfh);
	subdev_fh_free(subdev_fh);
	kfree(subdev_fh);
	file->private_data = NULL;

	return 0;
}

#if defined(CONFIG_VIDEO_V4L2_SUBDEV_API)
static int check_format(struct v4l2_subdev *sd,
			struct v4l2_subdev_format *format)
{
	if (format->which != V4L2_SUBDEV_FORMAT_TRY &&
	    format->which != V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EINVAL;

	if (format->pad >= sd->entity.num_pads)
		return -EINVAL;

	if (!(sd->flags & V4L2_SUBDEV_FL_HAS_SUBSTREAMS) && format->stream)
		return -EINVAL;

	return 0;
}

static int check_crop(struct v4l2_subdev *sd, struct v4l2_subdev_crop *crop)
{
	if (crop->which != V4L2_SUBDEV_FORMAT_TRY &&
	    crop->which != V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EINVAL;

	if (crop->pad >= sd->entity.num_pads)
		return -EINVAL;

	return 0;
}

static int check_selection(struct v4l2_subdev *sd,
			   struct v4l2_subdev_selection *sel)
{
	if (sel->which != V4L2_SUBDEV_FORMAT_TRY &&
	    sel->which != V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EINVAL;

	if (sel->pad >= sd->entity.num_pads)
		return -EINVAL;

	if (!(sd->flags & V4L2_SUBDEV_FL_HAS_SUBSTREAMS) && sel->stream)
		return -EINVAL;

	return 0;
}

static int check_edid(struct v4l2_subdev *sd, struct v4l2_subdev_edid *edid)
{
	if (edid->pad >= sd->entity.num_pads)
		return -EINVAL;

	if (edid->blocks && edid->edid == NULL)
		return -EINVAL;

	return 0;
}
#endif

static long subdev_do_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	struct v4l2_fh *vfh = file->private_data;
#if defined(CONFIG_VIDEO_V4L2_SUBDEV_API)
	struct v4l2_subdev_fh *subdev_fh = to_v4l2_subdev_fh(vfh);
	int rval;
#endif

	switch (cmd) {
	case VIDIOC_QUERYCTRL:
		return v4l2_queryctrl(vfh->ctrl_handler, arg);

	case VIDIOC_QUERY_EXT_CTRL:
		return v4l2_query_ext_ctrl(vfh->ctrl_handler, arg);

	case VIDIOC_QUERYMENU:
		return v4l2_querymenu(vfh->ctrl_handler, arg);

	case VIDIOC_G_CTRL:
		return v4l2_g_ctrl(vfh->ctrl_handler, arg);

	case VIDIOC_S_CTRL:
		return v4l2_s_ctrl(vfh, vfh->ctrl_handler, arg);

	case VIDIOC_G_EXT_CTRLS:
		return v4l2_g_ext_ctrls(vfh->ctrl_handler, arg);

	case VIDIOC_S_EXT_CTRLS:
		return v4l2_s_ext_ctrls(vfh, vfh->ctrl_handler, arg);

	case VIDIOC_TRY_EXT_CTRLS:
		return v4l2_try_ext_ctrls(vfh->ctrl_handler, arg);

	case VIDIOC_DQEVENT:
		if (!(sd->flags & V4L2_SUBDEV_FL_HAS_EVENTS))
			return -ENOIOCTLCMD;

		return v4l2_event_dequeue(vfh, arg, file->f_flags & O_NONBLOCK);

	case VIDIOC_SUBSCRIBE_EVENT:
		return v4l2_subdev_call(sd, core, subscribe_event, vfh, arg);

	case VIDIOC_UNSUBSCRIBE_EVENT:
		return v4l2_subdev_call(sd, core, unsubscribe_event, vfh, arg);

#ifdef CONFIG_VIDEO_ADV_DEBUG
	case VIDIOC_DBG_G_REGISTER:
	{
		struct v4l2_dbg_register *p = arg;

		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		return v4l2_subdev_call(sd, core, g_register, p);
	}
	case VIDIOC_DBG_S_REGISTER:
	{
		struct v4l2_dbg_register *p = arg;

		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		return v4l2_subdev_call(sd, core, s_register, p);
	}
#endif

	case VIDIOC_LOG_STATUS: {
		int ret;

		pr_info("%s: =================  START STATUS  =================\n",
			sd->name);
		ret = v4l2_subdev_call(sd, core, log_status);
		pr_info("%s: ==================  END STATUS  ==================\n",
			sd->name);
		return ret;
	}

#if defined(CONFIG_VIDEO_V4L2_SUBDEV_API)
	case VIDIOC_SUBDEV_G_FMT: {
		struct v4l2_subdev_format *format = arg;

		rval = check_format(sd, format);
		if (rval)
			return rval;

		return v4l2_subdev_call(sd, pad, get_fmt, subdev_fh->pad, format);
	}

	case VIDIOC_SUBDEV_S_FMT: {
		struct v4l2_subdev_format *format = arg;

		rval = check_format(sd, format);
		if (rval)
			return rval;

		return v4l2_subdev_call(sd, pad, set_fmt, subdev_fh->pad, format);
	}

	case VIDIOC_SUBDEV_G_CROP: {
		struct v4l2_subdev_crop *crop = arg;
		struct v4l2_subdev_selection sel;

		rval = check_crop(sd, crop);
		if (rval)
			return rval;

		memset(&sel, 0, sizeof(sel));
		sel.which = crop->which;
		sel.pad = crop->pad;
		sel.target = V4L2_SEL_TGT_CROP;

		rval = v4l2_subdev_call(
			sd, pad, get_selection, subdev_fh->pad, &sel);

		crop->rect = sel.r;

		return rval;
	}

	case VIDIOC_SUBDEV_S_CROP: {
		struct v4l2_subdev_crop *crop = arg;
		struct v4l2_subdev_selection sel;

		rval = check_crop(sd, crop);
		if (rval)
			return rval;

		memset(&sel, 0, sizeof(sel));
		sel.which = crop->which;
		sel.pad = crop->pad;
		sel.target = V4L2_SEL_TGT_CROP;
		sel.r = crop->rect;

		rval = v4l2_subdev_call(
			sd, pad, set_selection, subdev_fh->pad, &sel);

		crop->rect = sel.r;

		return rval;
	}

	case VIDIOC_SUBDEV_ENUM_MBUS_CODE: {
		struct v4l2_subdev_mbus_code_enum *code = arg;

		if (code->which != V4L2_SUBDEV_FORMAT_TRY &&
		    code->which != V4L2_SUBDEV_FORMAT_ACTIVE)
			return -EINVAL;

		if (code->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, pad, enum_mbus_code, subdev_fh->pad,
					code);
	}

	case VIDIOC_SUBDEV_ENUM_FRAME_SIZE: {
		struct v4l2_subdev_frame_size_enum *fse = arg;

		if (fse->which != V4L2_SUBDEV_FORMAT_TRY &&
		    fse->which != V4L2_SUBDEV_FORMAT_ACTIVE)
			return -EINVAL;

		if (fse->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, pad, enum_frame_size, subdev_fh->pad,
					fse);
	}

	case VIDIOC_SUBDEV_G_FRAME_INTERVAL: {
		struct v4l2_subdev_frame_interval *fi = arg;

		if (fi->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, video, g_frame_interval, arg);
	}

	case VIDIOC_SUBDEV_S_FRAME_INTERVAL: {
		struct v4l2_subdev_frame_interval *fi = arg;

		if (fi->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, video, s_frame_interval, arg);
	}

	case VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL: {
		struct v4l2_subdev_frame_interval_enum *fie = arg;

		if (fie->which != V4L2_SUBDEV_FORMAT_TRY &&
		    fie->which != V4L2_SUBDEV_FORMAT_ACTIVE)
			return -EINVAL;

		if (fie->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, pad, enum_frame_interval, subdev_fh->pad,
					fie);
	}

	case VIDIOC_SUBDEV_G_SELECTION: {
		struct v4l2_subdev_selection *sel = arg;

		rval = check_selection(sd, sel);
		if (rval)
			return rval;

		return v4l2_subdev_call(
			sd, pad, get_selection, subdev_fh->pad, sel);
	}

	case VIDIOC_SUBDEV_S_SELECTION: {
		struct v4l2_subdev_selection *sel = arg;

		rval = check_selection(sd, sel);
		if (rval)
			return rval;

		return v4l2_subdev_call(
			sd, pad, set_selection, subdev_fh->pad, sel);
	}

	case VIDIOC_G_EDID: {
		struct v4l2_subdev_edid *edid = arg;

		rval = check_edid(sd, edid);
		if (rval)
			return rval;

		return v4l2_subdev_call(sd, pad, get_edid, edid);
	}

	case VIDIOC_S_EDID: {
		struct v4l2_subdev_edid *edid = arg;

		rval = check_edid(sd, edid);
		if (rval)
			return rval;

		return v4l2_subdev_call(sd, pad, set_edid, edid);
	}

	case VIDIOC_SUBDEV_DV_TIMINGS_CAP: {
		struct v4l2_dv_timings_cap *cap = arg;

		if (cap->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, pad, dv_timings_cap, cap);
	}

	case VIDIOC_SUBDEV_ENUM_DV_TIMINGS: {
		struct v4l2_enum_dv_timings *dvt = arg;

		if (dvt->pad >= sd->entity.num_pads)
			return -EINVAL;

		return v4l2_subdev_call(sd, pad, enum_dv_timings, dvt);
	}

	case VIDIOC_SUBDEV_QUERY_DV_TIMINGS:
		return v4l2_subdev_call(sd, video, query_dv_timings, arg);

	case VIDIOC_SUBDEV_G_DV_TIMINGS:
		return v4l2_subdev_call(sd, video, g_dv_timings, arg);

	case VIDIOC_SUBDEV_S_DV_TIMINGS:
		return v4l2_subdev_call(sd, video, s_dv_timings, arg);

	case VIDIOC_SUBDEV_G_ROUTING:
		return v4l2_subdev_call(sd, pad, get_routing, arg);

	case VIDIOC_SUBDEV_S_ROUTING: {
		struct v4l2_subdev_routing *route = arg;
		unsigned int i;
		int rval;

		if (route->num_routes > sd->entity.num_pads)
			return -EINVAL;

		for (i = 0; i < route->num_routes; ++i) {
			unsigned int sink = route->routes[i].sink_pad;
			unsigned int source = route->routes[i].source_pad;
			struct media_pad *pads = sd->entity.pads;

			if (sink >= sd->entity.num_pads ||
			    source >= sd->entity.num_pads)
				return -EINVAL;

			if ((!(route->routes[i].flags &
				V4L2_SUBDEV_ROUTE_FL_SOURCE) &&
			    !(pads[sink].flags & MEDIA_PAD_FL_SINK)) ||
			    !(pads[source].flags & MEDIA_PAD_FL_SOURCE))
				return -EINVAL;
		}

		mutex_lock(&sd->entity.graph_obj.mdev->graph_mutex);
		rval = v4l2_subdev_call(sd, pad, set_routing, route);
		mutex_unlock(&sd->entity.graph_obj.mdev->graph_mutex);

		return rval;
	}
#endif

	default:
		return v4l2_subdev_call(sd, core, ioctl, cmd, arg);
	}

	return 0;
}

static long subdev_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	return video_usercopy(file, cmd, arg, subdev_do_ioctl);
}

#ifdef CONFIG_COMPAT
static long subdev_compat_ioctl32(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);

	return v4l2_subdev_call(sd, core, compat_ioctl32, cmd, arg);
}
#endif

static unsigned int subdev_poll(struct file *file, poll_table *wait)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	struct v4l2_fh *fh = file->private_data;

	if (!(sd->flags & V4L2_SUBDEV_FL_HAS_EVENTS))
		return POLLERR;

	poll_wait(file, &fh->wait, wait);

	if (v4l2_event_pending(fh))
		return POLLPRI;

	return 0;
}

const struct v4l2_file_operations v4l2_subdev_fops = {
	.owner = THIS_MODULE,
	.open = subdev_open,
	.unlocked_ioctl = subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = subdev_compat_ioctl32,
#endif
	.release = subdev_close,
	.poll = subdev_poll,
};

#ifdef CONFIG_MEDIA_CONTROLLER
int v4l2_subdev_link_validate_default(struct v4l2_subdev *sd,
				      struct media_link *link,
				      struct v4l2_subdev_format *source_fmt,
				      struct v4l2_subdev_format *sink_fmt)
{
	/* The width, height and code must match. */
	if (source_fmt->format.width != sink_fmt->format.width
	    || source_fmt->format.height != sink_fmt->format.height
	    || source_fmt->format.code != sink_fmt->format.code)
		return -EPIPE;

	/* The field order must match, or the sink field order must be NONE
	 * to support interlaced hardware connected to bridges that support
	 * progressive formats only.
	 */
	if (source_fmt->format.field != sink_fmt->format.field &&
	    sink_fmt->format.field != V4L2_FIELD_NONE)
		return -EPIPE;

	if (source_fmt->stream != sink_fmt->stream)
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL_GPL(v4l2_subdev_link_validate_default);

static int
v4l2_subdev_link_validate_get_format(struct media_pad *pad,
				     struct v4l2_subdev_format *fmt)
{
	if (is_media_entity_v4l2_subdev(pad->entity)) {
		struct v4l2_subdev *sd =
			media_entity_to_v4l2_subdev(pad->entity);

		fmt->which = V4L2_SUBDEV_FORMAT_ACTIVE;
		fmt->pad = pad->index;
		return v4l2_subdev_call(sd, pad, get_fmt, NULL, fmt);
	}

	WARN(pad->entity->function != MEDIA_ENT_F_IO_V4L,
	     "Driver bug! Wrong media entity type 0x%08x, entity %s\n",
	     pad->entity->function, pad->entity->name);

	return -EINVAL;
}

static int v4l2_subdev_link_validate_one(struct media_link *link,
		struct media_pad *source_pad, unsigned int source_stream,
		struct media_pad *sink_pad, unsigned int sink_stream)
{
	struct v4l2_subdev *sink;
	struct v4l2_subdev_format sink_fmt, source_fmt;
	int rval;

	source_fmt.stream = source_stream;
	rval = v4l2_subdev_link_validate_get_format(source_pad, &source_fmt);
	if (rval < 0)
		return 0;

	sink_fmt.stream = sink_stream;
	rval = v4l2_subdev_link_validate_get_format(sink_pad, &sink_fmt);
	if (rval < 0)
		return 0;

	sink = media_entity_to_v4l2_subdev(link->sink->entity);

	rval = v4l2_subdev_call(sink, pad, link_validate, link,
				&source_fmt, &sink_fmt);
	if (rval != -ENOIOCTLCMD)
		return rval;

	return v4l2_subdev_link_validate_default(
		sink, link, &source_fmt, &sink_fmt);
}

/* How many routes to assume there can be per a sub-device? */
#define LINK_VALIDATE_ROUTES	8

int v4l2_subdev_link_validate(struct media_link *link)
{
	struct v4l2_subdev *sink;
	struct v4l2_subdev_route sink_routes[LINK_VALIDATE_ROUTES];
	struct v4l2_subdev_routing sink_routing = {
		.routes = sink_routes,
		.num_routes = ARRAY_SIZE(sink_routes),
	};
	struct v4l2_subdev_route src_routes[LINK_VALIDATE_ROUTES];
	struct v4l2_subdev_routing src_routing = {
		.routes = src_routes,
		.num_routes = ARRAY_SIZE(src_routes),
	};
	unsigned int i, j;
	int rval;

	sink = media_entity_to_v4l2_subdev(link->sink->entity);

	if (!(link->sink->flags & MEDIA_PAD_FL_MULTIPLEX &&
		link->source->flags & MEDIA_PAD_FL_MULTIPLEX))
		return v4l2_subdev_link_validate_one(link, link->source, 0,
						     link->sink, 0);
	/*
	 * multiplex link cannot proceed without route information.
	 */
	rval = v4l2_subdev_call(sink, pad, get_routing, &sink_routing);

	if (rval) {
		dev_err(sink->entity.graph_obj.mdev->dev,
			"error %d in get_routing() on %s, sink pad %u\n", rval,
			sink->entity.name, link->sink->index);

		return rval;
	}

	rval = v4l2_subdev_call(media_entity_to_v4l2_subdev(
					link->source->entity),
				pad, get_routing, &src_routing);
	if (rval) {
		dev_dbg(sink->entity.graph_obj.mdev->dev,
			"error %d in get_routing() on %s, source pad %u\n",
			rval, sink->entity.name, link->source->index);

		return rval;
	}

	dev_dbg(sink->entity.graph_obj.mdev->dev,
		"validating multiplexed link \"%s\":%u -> \"%s\":%u; %u/%u routes\n",
		link->source->entity->name, link->source->index,
		sink->entity.name, link->sink->index,
		src_routing.num_routes, sink_routing.num_routes);

	for (i = 0; i < sink_routing.num_routes; i++) {
		/* Get the first active route for the sink pad. */
		if (sink_routes[i].sink_pad != link->sink->index ||
		    !(sink_routes[i].flags & V4L2_SUBDEV_ROUTE_FL_ACTIVE)) {
			dev_dbg(sink->entity.graph_obj.mdev->dev,
				"skipping sink route %u/%u -> %u/%u[%u]\n",
				sink_routes[i].sink_pad,
				sink_routes[i].sink_stream,
				sink_routes[i].source_pad,
				sink_routes[i].source_stream,
				(bool)(sink_routes[i].flags
				       & V4L2_SUBDEV_ROUTE_FL_ACTIVE));
			continue;
		}

		/*
		 * Get the corresponding route for the source pad.
		 * It's ok for the source pad to have routes active
		 * where the sink pad does not, but the routes that
		 * are active on the source pad have to be active on
		 * the sink pad as well.
		 */

		for (j = 0; j < src_routing.num_routes; j++) {
			if (src_routes[j].source_pad == link->source->index &&
			    src_routes[j].source_stream
			    == sink_routes[i].sink_stream)
				break;
		}

		if (j == src_routing.num_routes) {
			dev_err(sink->entity.graph_obj.mdev->dev,
				"no corresponding source found.\n");
			return -EINVAL;
		}

		/* The source route must be active. */
		if (!(src_routes[j].flags & V4L2_SUBDEV_ROUTE_FL_ACTIVE)) {
			dev_dbg(sink->entity.graph_obj.mdev->dev,
				"source route not active\n");
			return -EINVAL;
		}

		dev_dbg(sink->entity.graph_obj.mdev->dev,
			"validating link \"%s\": %u/%u => \"%s\" %u/%u\n",
			link->source->entity->name, src_routes[j].source_pad,
			src_routes[j].source_stream, sink->entity.name,
			sink_routes[i].sink_pad, sink_routes[i].sink_stream);

		rval = v4l2_subdev_link_validate_one(
			link, link->source, src_routes[j].source_stream,
			link->sink, sink_routes[j].sink_stream);
		if (rval) {
			dev_dbg(sink->entity.graph_obj.mdev->dev,
				"error %d in link validation\n", rval);
			return rval;
		}
	}

	if (i < sink_routing.num_routes) {
		dev_dbg(sink->entity.graph_obj.mdev->dev,
			"not all sink routes verified; out of source routes\n");
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(v4l2_subdev_link_validate);

struct v4l2_subdev_pad_config *
v4l2_subdev_alloc_pad_config(struct v4l2_subdev *sd)
{
	struct v4l2_subdev_pad_config *cfg;
	int ret;

	if (!sd->entity.num_pads)
		return NULL;

	cfg = kcalloc(sd->entity.num_pads, sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return NULL;

	ret = v4l2_subdev_call(sd, pad, init_cfg, cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		kfree(cfg);
		return NULL;
	}

	return cfg;
}
EXPORT_SYMBOL_GPL(v4l2_subdev_alloc_pad_config);

void v4l2_subdev_free_pad_config(struct v4l2_subdev_pad_config *cfg)
{
	kfree(cfg);
}
EXPORT_SYMBOL_GPL(v4l2_subdev_free_pad_config);
#endif /* CONFIG_MEDIA_CONTROLLER */

void v4l2_subdev_init(struct v4l2_subdev *sd, const struct v4l2_subdev_ops *ops)
{
	INIT_LIST_HEAD(&sd->list);
	BUG_ON(!ops);
	sd->ops = ops;
	sd->v4l2_dev = NULL;
	sd->flags = 0;
	sd->name[0] = '\0';
	sd->grp_id = 0;
	sd->dev_priv = NULL;
	sd->host_priv = NULL;
#if defined(CONFIG_MEDIA_CONTROLLER)
	sd->entity.name = sd->name;
	sd->entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	sd->entity.function = MEDIA_ENT_F_V4L2_SUBDEV_UNKNOWN;
#endif
}
EXPORT_SYMBOL(v4l2_subdev_init);

void v4l2_subdev_notify_event(struct v4l2_subdev *sd,
			      const struct v4l2_event *ev)
{
	v4l2_event_queue(sd->devnode, ev);
	v4l2_subdev_notify(sd, V4L2_DEVICE_NOTIFY_EVENT, (void *)ev);
}
EXPORT_SYMBOL_GPL(v4l2_subdev_notify_event);
