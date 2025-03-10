/***************************************************************************
 *            friend.c
 *
 *  Sat May 15 15:25:16 2004
 *  Copyright  2004-2009  Simon Morlat
 *  Email
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "linphonecore.h"
#include "private.h"
#include "lpconfig.h"

const char *linphone_online_status_to_string(LinphoneOnlineStatus ss){
	const char *str=NULL;
	switch(ss){
		case LINPHONE_STATUS_ONLINE:
		str=_("Online");
		break;
		case LINPHONE_STATUS_BUSY:
		str=_("Busy");
		break;
		case LINPHONE_STATUS_BERIGHTBACK:
		str=_("Be right back");
		break;
		case LINPHONE_STATUS_AWAY:
		str=_("Away");
		break;
		case LINPHONE_STATUS_ONTHEPHONE:
		str=_("On the phone");
		break;
		case LINPHONE_STATUS_OUTTOLUNCH:
		str=_("Out to lunch");
		break;
		case LINPHONE_STATUS_NOT_DISTURB:
		str=_("Do not disturb");
		break;
		case LINPHONE_STATUS_MOVED:
		str=_("Moved");
		break;
		case LINPHONE_STATUS_ALT_SERVICE:
		str=_("Using another messaging service");
		break;
		case LINPHONE_STATUS_OFFLINE:
		str=_("Offline");
		break;
		case LINPHONE_STATUS_PENDING:
		str=_("Pending");
		break;
		default:
		str=_("Unknown-bug");
	}
	return str;
}

static int friend_data_compare(const void * a, const void * b, void * data){
	LinphoneAddress *fa=((LinphoneFriend*)a)->uri;
	LinphoneAddress *fb=((LinphoneFriend*)b)->uri;
	const char *ua,*ub;
	ua=linphone_address_get_username(fa);
	ub=linphone_address_get_username(fb);
	if (ua!=NULL && ub!=NULL) {
		//printf("Comparing usernames %s,%s\n",ua,ub);
		return strcasecmp(ua,ub);
	}
	else {
		/* compare hosts*/
		ua=linphone_address_get_domain(fa);
		ub=linphone_address_get_domain(fb);
		if (ua!=NULL && ub!=NULL){
			int ret=strcasecmp(ua,ub);
			//printf("Comparing hostnames %s,%s,res=%i\n",ua,ub,ret);
			return ret;
		}
		else return -1;
	}
}

static int friend_compare(const void * a, const void * b){
	return friend_data_compare(a,b,NULL);
}


MSList *linphone_find_friend(MSList *fl, const LinphoneAddress *friend, LinphoneFriend **lf){
	MSList *res=NULL;
	LinphoneFriend dummy;
	if (lf!=NULL) *lf=NULL;
	dummy.uri=(LinphoneAddress*)friend;
	res=ms_list_find_custom(fl,friend_compare,&dummy);
	if (lf!=NULL && res!=NULL) *lf=(LinphoneFriend*)res->data;
	return res;
}

LinphoneFriend *linphone_find_friend_by_inc_subscribe(MSList *l, SalOp *op){
	MSList *elem;
	for (elem=l;elem!=NULL;elem=elem->next){
		LinphoneFriend *lf=(LinphoneFriend*)elem->data;
		if (lf->insub==op) return lf;
	}
	return NULL;
}

LinphoneFriend *linphone_find_friend_by_out_subscribe(MSList *l, SalOp *op){
	MSList *elem;
	for (elem=l;elem!=NULL;elem=elem->next){
		LinphoneFriend *lf=(LinphoneFriend*)elem->data;
		if (lf->outsub==op) return lf;
	}
	return NULL;
}

void __linphone_friend_do_subscribe(LinphoneFriend *fr){
	char *friend=NULL;
	const char *route=NULL;
	const char *from=NULL;
	LinphoneProxyConfig *cfg;
	
	friend=linphone_address_as_string(fr->uri);
	cfg=linphone_core_lookup_known_proxy(fr->lc,linphone_friend_get_address(fr));
	if (cfg!=NULL){
		route=linphone_proxy_config_get_route(cfg);
		from=linphone_proxy_config_get_identity(cfg);
	}else from=linphone_core_get_primary_contact(fr->lc);
	if (fr->outsub==NULL){
		/* people for which we don't have yet an answer should appear as offline */
		fr->status=LINPHONE_STATUS_OFFLINE;
		/*
		if (fr->lc->vtable.notify_recv)
			fr->lc->vtable.notify_recv(fr->lc,(LinphoneFriend*)fr);
		 */
	}else{
		sal_op_release(fr->outsub);
		fr->outsub=NULL;
	}
	fr->outsub=sal_op_new(fr->lc->sal);
	sal_op_set_route(fr->outsub,route);
	sal_subscribe_presence(fr->outsub,from,friend);
	fr->subscribe_active=TRUE;
	ms_free(friend);
}

LinphoneFriend * linphone_friend_new(){
	LinphoneFriend *obj=ms_new0(LinphoneFriend,1);
	obj->pol=LinphoneSPAccept;
	obj->status=LINPHONE_STATUS_OFFLINE;
	obj->subscribe=TRUE;
	return obj;	
}

LinphoneFriend *linphone_friend_new_with_addr(const char *addr){
	LinphoneFriend *fr=linphone_friend_new();
	if (linphone_friend_set_sip_addr(fr,addr)<0){
		linphone_friend_destroy(fr);
		return NULL;
	}
	return fr;
}

bool_t linphone_friend_in_list(const LinphoneFriend *lf){
	return lf->lc!=NULL;
}

void linphone_core_interpret_friend_uri(LinphoneCore *lc, const char *uri, char **result){
	LinphoneAddress *fr=NULL;
	*result=NULL;
	fr=linphone_address_new(uri);
	if (fr==NULL){
		char *tmp=NULL;
		if (strchr(uri,'@')!=NULL){
			LinphoneAddress *u;
			/*try adding sip:*/
			tmp=ms_strdup_printf("sip:%s",uri);
			u=linphone_address_new(tmp);
			if (u!=NULL){
				*result=tmp;
			}
		}else if (lc->default_proxy!=NULL){
			/*try adding domain part from default current proxy*/
			LinphoneAddress * id=linphone_address_new(linphone_core_get_identity(lc));
			if (id!=NULL){
				linphone_address_set_username(id,uri);
				*result=linphone_address_as_string(id);
				linphone_address_destroy(id);
			}
		}
		if (*result){
			/*looks good */
			ms_message("%s interpreted as %s",uri,*result);
		}else{
			ms_warning("Fail to interpret friend uri %s",uri);
		}
	}else *result=linphone_address_as_string(fr);
	linphone_address_destroy(fr);
}

int linphone_friend_set_sip_addr(LinphoneFriend *lf, const char *addr){
	LinphoneAddress *fr=linphone_address_new(addr);
	if (fr==NULL) {
		ms_warning("Invalid friend sip uri: %s",addr);
		return -1;
	}
	linphone_address_clean(fr);
	if (lf->uri!=NULL) linphone_address_destroy(lf->uri);	
	lf->uri=fr;
	return 0;
}

int linphone_friend_set_name(LinphoneFriend *lf, const char *name){
	LinphoneAddress *fr=lf->uri;
	if (fr==NULL){
		ms_error("linphone_friend_set_sip_addr() must be called before linphone_friend_set_name().");
		return -1;
	}
	linphone_address_set_display_name(fr,name);
	return 0;
}

int linphone_friend_send_subscribe(LinphoneFriend *fr, bool_t val){
	fr->subscribe=val;
	return 0;
}

int linphone_friend_set_inc_subscribe_policy(LinphoneFriend *fr, LinphoneSubscribePolicy pol)
{
	fr->pol=pol;
	return 0;
}

SalPresenceStatus linphone_online_status_to_sal(LinphoneOnlineStatus os){
	switch(os){
		case LINPHONE_STATUS_OFFLINE:
			return SalPresenceOffline;
		break;
		case LINPHONE_STATUS_ONLINE:
			return SalPresenceOnline;
		break;
		case LINPHONE_STATUS_BUSY:
			return SalPresenceBusy;
		break;
		case LINPHONE_STATUS_BERIGHTBACK:
			return SalPresenceBerightback;
		break;
		case LINPHONE_STATUS_AWAY:
			return SalPresenceAway;
		break;
		case LINPHONE_STATUS_ONTHEPHONE:
			return SalPresenceOnthephone;
		break;
		case LINPHONE_STATUS_OUTTOLUNCH:
			return SalPresenceOuttolunch;
		break;
		case LINPHONE_STATUS_NOT_DISTURB:
			return SalPresenceDonotdisturb;
		break;
		case LINPHONE_STATUS_MOVED:
			return SalPresenceMoved;
		break;
		case LINPHONE_STATUS_ALT_SERVICE:
			return SalPresenceAltService;
		break;
		case LINPHONE_STATUS_PENDING:
			return SalPresenceOffline;
		break;
		default:
			return SalPresenceOffline;
		break;
	}
	return SalPresenceOffline;
}

void linphone_friend_notify(LinphoneFriend *lf, LinphoneOnlineStatus os){
	//printf("Wish to notify %p, lf->nid=%i\n",lf,lf->nid);
	if (lf->insub!=NULL){
		sal_notify_presence(lf->insub,linphone_online_status_to_sal(os),NULL);
	}
}

static void linphone_friend_unsubscribe(LinphoneFriend *lf){
	if (lf->outsub!=NULL) {
		sal_unsubscribe(lf->outsub);
		sal_op_release(lf->outsub);
		lf->outsub=NULL;
		lf->subscribe_active=FALSE;
	}
}

void linphone_friend_close_subscriptions(LinphoneFriend *lf){
	linphone_friend_unsubscribe(lf);
	if (lf->insub){
		sal_notify_close(lf->insub);
		sal_op_release(lf->insub);
		lf->insub=NULL;
	}
}

void linphone_friend_destroy(LinphoneFriend *lf){
	
	if (lf->uri!=NULL) linphone_address_destroy(lf->uri);
	if (lf->info!=NULL) buddy_info_free(lf->info);
	ms_free(lf);
}

const LinphoneAddress *linphone_friend_get_address(const LinphoneFriend *lf){
	return lf->uri;
}

bool_t linphone_friend_get_send_subscribe(const LinphoneFriend *lf){
	return lf->subscribe;
}

LinphoneSubscribePolicy linphone_friend_get_inc_subscribe_policy(const LinphoneFriend *lf){
	return lf->pol;
}

LinphoneOnlineStatus linphone_friend_get_status(const LinphoneFriend *lf){
	return lf->status;
}

BuddyInfo * linphone_friend_get_info(const LinphoneFriend *lf){
	return lf->info;
}

void linphone_friend_apply(LinphoneFriend *fr, LinphoneCore *lc){
	if (fr->uri==NULL) {
		ms_warning("No sip url defined.");
		return;
	}
	fr->lc=lc;
	
	linphone_core_write_friends_config(lc);

	if (fr->inc_subscribe_pending){
		switch(fr->pol){
			case LinphoneSPWait:
				linphone_friend_notify(fr,LINPHONE_STATUS_PENDING);
				break;
			case LinphoneSPAccept:
				if (fr->lc!=NULL)
				  {
					linphone_friend_notify(fr,fr->lc->presence_mode);
				  }
				break;
			case LinphoneSPDeny:
				linphone_friend_notify(fr,LINPHONE_STATUS_OFFLINE);
				break;
		}
		fr->inc_subscribe_pending=FALSE;
	}
	if (fr->subscribe && fr->subscribe_active==FALSE){
		ms_message("Sending a new SUBSCRIBE");
		__linphone_friend_do_subscribe(fr);
	}
	ms_message("linphone_friend_apply() done.");
	lc->bl_refresh=TRUE;
}

void linphone_friend_edit(LinphoneFriend *fr){
}

void linphone_friend_done(LinphoneFriend *fr){
	ms_return_if_fail(fr!=NULL);
	if (fr->lc==NULL) return;
	linphone_friend_apply(fr,fr->lc);
}

void linphone_core_add_friend(LinphoneCore *lc, LinphoneFriend *lf)
{
	ms_return_if_fail(lf->lc==NULL);
	ms_return_if_fail(lf->uri!=NULL);
	if (ms_list_find(lc->friends,lf)!=NULL){
		char *tmp=NULL;
		const LinphoneAddress *addr=linphone_friend_get_address(lf);
		if (addr) tmp=linphone_address_as_string(addr);
		ms_warning("Friend %s already in list, ignored.", tmp ? tmp : "unknown");
		if (tmp) ms_free(tmp);
		return ;
	}
	lc->friends=ms_list_append(lc->friends,lf);
	linphone_friend_apply(lf,lc);
	return ;
}

void linphone_core_remove_friend(LinphoneCore *lc, LinphoneFriend* fl){
	MSList *el=ms_list_find(lc->friends,(void *)fl);
	if (el!=NULL){
		lc->friends=ms_list_remove_link(lc->friends,el);
		linphone_friend_destroy((LinphoneFriend*)el->data);
		linphone_core_write_friends_config(lc);
	}
}

void linphone_friend_set_ref_key(LinphoneFriend *lf, const char *key){
	if (lf->refkey!=NULL){
		ms_free(lf->refkey);
		lf->refkey=NULL;
	}
	if (key)
		lf->refkey=ms_strdup(key);
	if (lf->lc)
		linphone_core_write_friends_config(lf->lc);
}

const char *linphone_friend_get_ref_key(const LinphoneFriend *lf){
	return lf->refkey;
}

static bool_t username_match(const char *u1, const char *u2){
	if (u1==NULL && u2==NULL) return TRUE;
	if (u1 && u2 && strcasecmp(u1,u2)==0) return TRUE;
	return FALSE;
}

LinphoneFriend *linphone_core_get_friend_by_address(const LinphoneCore *lc, const char *uri){
	LinphoneAddress *puri=linphone_address_new(uri);
	const MSList *elem;
	const char *username=linphone_address_get_username(puri);
	const char *domain=linphone_address_get_domain(puri);
	LinphoneFriend *lf=NULL;
		
	if (puri==NULL){
		return NULL;
	}
	for(elem=lc->friends;elem!=NULL;elem=ms_list_next(elem)){
		lf=(LinphoneFriend*)elem->data;
		const char *it_username=linphone_address_get_username(lf->uri);
		const char *it_host=linphone_address_get_domain(lf->uri);;
		if (strcasecmp(domain,it_host)==0 && username_match(username,it_username)){
			break;
		}
		lf=NULL;
	}
	linphone_address_destroy(puri);
	return lf;
}

LinphoneFriend *linphone_core_get_friend_by_ref_key(const LinphoneCore *lc, const char *key){
	const MSList *elem;
	if (key==NULL) return NULL;
	for(elem=linphone_core_get_friend_list(lc);elem!=NULL;elem=elem->next){
		LinphoneFriend *lf=(LinphoneFriend*)elem->data;
		if (lf->refkey!=NULL && strcmp(lf->refkey,key)==0){
			return lf;
		}
	}
	return NULL;
}

#define key_compare(s1,s2)	strcmp(s1,s2)

LinphoneSubscribePolicy __policy_str_to_enum(const char* pol){
	if (key_compare("accept",pol)==0){
		return LinphoneSPAccept;
	}
	if (key_compare("deny",pol)==0){
		return LinphoneSPDeny;
	}
	if (key_compare("wait",pol)==0){
		return LinphoneSPWait;
	}
	ms_warning("Unrecognized subscribe policy: %s",pol);
	return LinphoneSPWait;
}

LinphoneProxyConfig *__index_to_proxy(LinphoneCore *lc, int index){
	if (index>=0) return (LinphoneProxyConfig*)ms_list_nth_data(lc->sip_conf.proxies,index);
	else return NULL;
}

LinphoneFriend * linphone_friend_new_from_config_file(LinphoneCore *lc, int index){
	const char *tmp;
	char item[50];
	int a;
	LinphoneFriend *lf;
	LpConfig *config=lc->config;
	
	sprintf(item,"friend_%i",index);
	
	if (!lp_config_has_section(config,item)){
		return NULL;
	}
	
	tmp=lp_config_get_string(config,item,"url",NULL);
	if (tmp==NULL) {
		return NULL;
	}
	lf=linphone_friend_new_with_addr(tmp);
	if (lf==NULL) {
		return NULL;
	}
	tmp=lp_config_get_string(config,item,"pol",NULL);
	if (tmp==NULL) linphone_friend_set_inc_subscribe_policy(lf,LinphoneSPWait);
	else{
		linphone_friend_set_inc_subscribe_policy(lf,__policy_str_to_enum(tmp));
	}
	a=lp_config_get_int(config,item,"subscribe",0);
	linphone_friend_send_subscribe(lf,a);
		
	linphone_friend_set_ref_key(lf,lp_config_get_string(config,item,"refkey",NULL));
	return lf;
}

const char *__policy_enum_to_str(LinphoneSubscribePolicy pol){
	switch(pol){
		case LinphoneSPAccept:
			return "accept";
			break;
		case LinphoneSPDeny:
			return "deny";
			break;
		case LinphoneSPWait:
			return "wait";
			break;
	}
	ms_warning("Invalid policy enum value.");
	return "wait";
}

void linphone_friend_write_to_config_file(LpConfig *config, LinphoneFriend *lf, int index){
	char key[50];
	char *tmp;
	const char *refkey;
	
	sprintf(key,"friend_%i",index);
	
	if (lf==NULL){
		lp_config_clean_section(config,key);
		return;
	}
	if (lf->uri!=NULL){
		tmp=linphone_address_as_string(lf->uri);
		if (tmp==NULL) {
			return;
		}
		lp_config_set_string(config,key,"url",tmp);
		ms_free(tmp);
	}
	lp_config_set_string(config,key,"pol",__policy_enum_to_str(lf->pol));
	lp_config_set_int(config,key,"subscribe",lf->subscribe);

	refkey=linphone_friend_get_ref_key(lf);
	if (refkey){
		lp_config_set_string(config,key,"refkey",refkey);
	}
}

void linphone_core_write_friends_config(LinphoneCore* lc)
{
	MSList *elem;
	int i;
	if (!lc->ready) return; /*dont write config when reading it !*/
	for (elem=lc->friends,i=0; elem!=NULL; elem=ms_list_next(elem),i++){
		linphone_friend_write_to_config_file(lc->config,(LinphoneFriend*)elem->data,i);
	}
	linphone_friend_write_to_config_file(lc->config,NULL,i);	/* set the end */
}

