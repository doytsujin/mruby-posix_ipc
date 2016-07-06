/*
** mrb_pmq.c - PMQ class
**
** Copyright (c) Uchio Kondo 2016
**
** See Copyright Notice in LICENSE
*/

#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <mqueue.h>

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mrb_posix_ipc.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  struct mq_attr *attr;
  mqd_t mqd;
  char *name;
  bool unlinked;
} mrb_pmq_data;

static void mrb_pmq_free(mrb_state *mrb, void *p)
{
  mrb_pmq_data *d = (mrb_pmq_data *)p;
  mq_close(d->mqd);
  free(d->attr);
  mrb_free(mrb, d);
}

static const struct mrb_data_type mrb_pmq_data_type = {
  "mrb_pmq_data", mrb_pmq_free,
};

static mrb_value mrb_pmq_init(mrb_state *mrb, mrb_value self)
{
  mrb_pmq_data *data;
  char *name;
  mrb_int flag;
  struct mq_attr *attr;

  data = (mrb_pmq_data *)DATA_PTR(self);
  if (data) {
    mrb_free(mrb, data);
  }
  DATA_TYPE(self) = &mrb_pmq_data_type;
  DATA_PTR(self) = NULL;

  mrb_get_args(mrb, "zi", &name, &flag);
  data = (mrb_pmq_data *)mrb_malloc(mrb, sizeof(mrb_pmq_data));
  attr = (struct mq_attr *)malloc(sizeof(struct mq_attr));
  /* TODO: to be configurable */
  attr->mq_flags = 0;
  attr->mq_maxmsg = 1;
  attr->mq_msgsize = 100;
  attr->mq_curmsgs = 0;

  data->mqd = mq_open(name, (int)flag, 0644, attr);
  if(data->mqd < 0) {
    mrb_sys_fail(mrb, "mq_open failed.");
  }
  data->attr = attr;
  data->name = name;
  data->unlinked = false;
  DATA_PTR(self) = data;

  return self;
}

static mrb_value mrb_pmq_send(mrb_state *mrb, mrb_value self)
{
  mrb_pmq_data *data = (mrb_pmq_data *)DATA_PTR(self);
  char *msg;
  mrb_int len, prio = 0;

  mrb_get_args(mrb, "s|i", &msg, &len, &prio);
  if(data->attr->mq_msgsize < len) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "message size exceeded mq max size.");
  }
  if(mq_send(data->mqd, msg, len, prio) < 0){
    mrb_sys_fail(mrb, "mq_send failed.");
  }

  return mrb_str_new(mrb, msg, len);
}

static mrb_value mrb_pmq_receive(mrb_state *mrb, mrb_value self)
{
  mrb_pmq_data *data = (mrb_pmq_data *)DATA_PTR(self);
  char *buf;
  ssize_t read_buf;
  buf = (char *)malloc(data->attr->mq_msgsize * sizeof(char));

  read_buf = mq_receive(data->mqd, buf, data->attr->mq_msgsize, NULL);
  if(read_buf < 0) {
    mrb_sys_fail(mrb, "mq_receive failed.");
  }

  return mrb_str_new(mrb, buf, read_buf);
}

static mrb_value mrb_pmq_close(mrb_state *mrb, mrb_value self)
{
  mrb_pmq_data *data = (mrb_pmq_data *)DATA_PTR(self);

  return mrb_fixnum_value(mq_close(data->mqd));
}

static mrb_value mrb_pmq_unlink(mrb_state *mrb, mrb_value self)
{
  mrb_pmq_data *data = (mrb_pmq_data *)DATA_PTR(self);
  int ret;

  ret = mq_unlink(data->name);
  if(ret < 0) {
    mrb_sys_fail(mrb, "mq_unlink failed.");
  }
  data->unlinked = true;
  return mrb_fixnum_value(ret);
}

static mrb_value mrb_pmq_name(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new_cstr(mrb, ((mrb_pmq_data *) DATA_PTR(self))->name);
}

static mrb_value mrb_pmq_is_unlinked(mrb_state *mrb, mrb_value self)
{
  return mrb_bool_value(((mrb_pmq_data *) DATA_PTR(self))->unlinked);
}

void mrb_mruby_posix_ipc_gem_init(mrb_state *mrb)
{
  struct RClass *pmq;
  pmq = mrb_define_class(mrb, "PMQ", mrb->object_class);
  mrb_define_method(mrb, pmq, "initialize", mrb_pmq_init,        MRB_ARGS_REQ(2));
  mrb_define_method(mrb, pmq, "send",       mrb_pmq_send,        MRB_ARGS_ARG(1,1));
  mrb_define_method(mrb, pmq, "receive",    mrb_pmq_receive,     MRB_ARGS_NONE());
  mrb_define_method(mrb, pmq, "close",      mrb_pmq_close,       MRB_ARGS_NONE());
  mrb_define_method(mrb, pmq, "unlink",     mrb_pmq_unlink,      MRB_ARGS_NONE());
  mrb_define_method(mrb, pmq, "name",       mrb_pmq_name,        MRB_ARGS_NONE());
  mrb_define_method(mrb, pmq, "unlinked?",  mrb_pmq_is_unlinked, MRB_ARGS_NONE());

  /* Flags for open file mode */
  mrb_define_const(mrb, pmq, "O_CREAT",  mrb_fixnum_value(O_CREAT));
  mrb_define_const(mrb, pmq, "O_RDONLY", mrb_fixnum_value(O_RDONLY));
  mrb_define_const(mrb, pmq, "O_WRONLY", mrb_fixnum_value(O_WRONLY));
  mrb_define_const(mrb, pmq, "O_RDWR",   mrb_fixnum_value(O_RDWR));

  DONE;
}

void mrb_mruby_posix_ipc_gem_final(mrb_state *mrb)
{
}