
void setGenericCommand(client *c, int flags, robj *key, robj *val, robj *expire, int unit, robj *ok_reply, robj *abort_reply) {
    long long milliseconds = 0; /* initialized to avoid any harmness warning */

    if (expire) {
        if (getLongLongFromObjectOrReply(c, expire, &milliseconds, NULL) != C_OK)
            return;
        if (milliseconds <= 0) {
            addReplyErrorFormat(c,"invalid expire time in %s",c->cmd->name);
            return;
        }
        if (unit == UNIT_SECONDS) milliseconds *= 1000;
    }

    if ((flags & OBJ_SET_NX && lookupKeyWrite(c->db,key) != NULL) ||
        (flags & OBJ_SET_XX && lookupKeyWrite(c->db,key) == NULL))
    {
        addReply(c, abort_reply ? abort_reply : shared.null[c->resp]);
        return;
    }
    genericSetKey(c,c->db,key,val,flags & OBJ_SET_KEEPTTL,1);

    //mybegin
   // FILE *fp;
   // fp = fopen("my.txt", "a");
   // //fprintf(stdout, "val:%s\n", (char*)(val->ptr));
   // fprintf(fp, "val:%s\n", (char*)(val->ptr));
   // fclose(fp);
    //myend

    size_t valSize;
    char* cmdCont = (char*)(c->argv[0]->ptr);
    char* keyCont = (char*)(key->ptr);
    FILE* f;
    f = fopen("op.txt", "a");
    //f = stdout;
    if (f == NULL) {
	serverLog(LL_NOTICE, "file open fail!");
	exit(1);
    }
    if (val->encoding == OBJ_ENCODING_INT) {
	valSize = 8;
	long valCont = (long)(val->ptr);
	fprintf(f, "valSize:%lu, command:%s,%s,%ld\n", valSize, cmdCont, keyCont, valCont);
    }
    else {
	valSize = sdslen(val->ptr);
	char* valCont = (char*)(val->ptr);
	fprintf(f, "valSize:%lu, command:%s,%s,%s\n", valSize, cmdCont, keyCont, valCont);
    }

    fclose(f);



    server.dirty++;
    if (expire) setExpire(c,c->db,key,mstime()+milliseconds);
    notifyKeyspaceEvent(NOTIFY_STRING,"set",key,c->db->id);
    if (expire) notifyKeyspaceEvent(NOTIFY_GENERIC,
        "expire",key,c->db->id);
    addReply(c, ok_reply ? ok_reply : shared.ok);
}
