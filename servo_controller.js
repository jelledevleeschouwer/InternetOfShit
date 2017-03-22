#!/usr/bin/env node
/**
 *
 **/

init_servod();
toilet_flush(95);
aroma_diffuser(1);
aroma_diffuser(2);
toilet_close();
toilet_lock();
toilet_unlock();
kill_servod();

