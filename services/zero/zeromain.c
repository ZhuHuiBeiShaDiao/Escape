/**
 * $Id$
 * Copyright (C) 2008 - 2009 Nils Asmussen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <esc/common.h>
#include <esc/service.h>
#include <esc/io.h>
#include <esc/fileio.h>
#include <stdlib.h>
#include <messages.h>
#include <errors.h>

static sMsg msg;

int main(void) {
	tServ id;
	tMsgId mid;

	id = regService("zero",SERV_DRIVER);
	if(id < 0)
		error("Unable to register service 'zero'");

	/* 0's are always available ;) */
	if(setDataReadable(id,true) < 0)
		error("setDataReadable");

    /* wait for commands */
	while(1) {
		tFD fd = getWork(&id,1,NULL,&mid,&msg,sizeof(msg),0);
		if(fd < 0)
			printe("[ZERO] Unable to get work");
		else {
			switch(mid) {
				case MSG_DRV_OPEN:
					msg.args.arg1 = 0;
					send(fd,MSG_DRV_OPEN_RESP,&msg,sizeof(msg.args));
					break;
				case MSG_DRV_READ: {
					/* offset is ignored here */
					u32 count = msg.args.arg2;
					u8 *data = (u8*)calloc(count,sizeof(u8));
					if(!data)
						count = 0;
					msg.args.arg1 = count;
					msg.args.arg2 = true;
					send(fd,MSG_DRV_READ_RESP,&msg,sizeof(msg.args));
					if(data) {
						send(fd,MSG_DRV_READ_RESP,data,count);
						free(data);
					}
				}
				break;
				case MSG_DRV_WRITE:
					msg.args.arg1 = ERR_UNSUPPORTED_OP;
					send(fd,MSG_DRV_WRITE_RESP,&msg,sizeof(msg.args));
					break;
				case MSG_DRV_IOCTL: {
					msg.data.arg1 = ERR_UNSUPPORTED_OP;
					send(fd,MSG_DRV_IOCTL_RESP,&msg,sizeof(msg.data));
				}
				break;
				case MSG_DRV_CLOSE:
					break;
			}
			close(fd);
		}
	}

	/* clean up */
	unregService(id);
	return EXIT_SUCCESS;
}
