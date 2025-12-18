<template>
  <div class="page">
    <el-row :gutter="16">
      <el-col
        :xs="24"
        :md="12"
      >
        <SystemStatusCard
          :loading="loading"
          :ctl-busy="ctlBusy"
          :status="status"
          :running="isFilesrvRunning"
          :pid="ctl.pid"
          @refresh="refreshAll"
          @start="startSrv"
          @stop="stopSrv"
        />
      </el-col>

      <el-col
        :xs="24"
        :md="12"
      >
        <InboxFilesCard
          :loading="filesLoading"
          :files="files"
          @refresh="refreshFiles"
        />
      </el-col>
    </el-row>

    <LastOperationCard
      :last-msg="lastMsg"
      :last-raw="lastRaw"
    />
  </div>
</template>

<script setup lang="ts">
import { api, type StatusResp, type FileItem, type CtlResp } from "@/api";
import SystemStatusCard from "@/components/SystemStatusCard.vue";
import InboxFilesCard from "@/components/InboxFilesCard.vue";
import LastOperationCard from "@/components/LastOperationCard.vue";

const loading = ref(false);
const filesLoading = ref(false);
const ctlBusy = ref(false);

const status = ref<StatusResp | null>(null);
const files = ref<FileItem[]>([]);
const ctl = ref<CtlResp>({ ok: true, msg: "init", running: false });

const lastMsg = ref("");
const lastRaw = ref("");

const isFilesrvRunning = computed(() => !!ctl.value.running);

function recordOp(title: string, obj: any) {
  lastMsg.value = title;
  lastRaw.value = JSON.stringify(obj, null, 2);
}

async function refreshStatus() {
  status.value = await api.status();
}

async function refreshFiles() {
  filesLoading.value = true;
  try {
    files.value = await api.files();
  } finally {
    filesLoading.value = false;
  }
}

async function refreshCtl() {
  ctl.value = await api.ctlStatus();
}

async function refreshAll() {
  loading.value = true;
  try {
    await Promise.all([refreshStatus(), refreshFiles(), refreshCtl()]);
  } catch (e: any) {
    ElMessage.error(e?.message ?? "refresh failed");
  } finally {
    loading.value = false;
  }
}

async function startSrv() {
  ctlBusy.value = true;
  try {
    const r = await api.ctlStart();
    ctl.value = r;
    recordOp("Start filesrv", r);
    ElMessage.success(r.msg);
  } catch (e: any) {
    ElMessage.error(e?.message ?? "start failed");
  } finally {
    ctlBusy.value = false;
  }
}

async function stopSrv() {
  ctlBusy.value = true;
  try {
    const r = await api.ctlStop();
    ctl.value = r;
    recordOp("Stop filesrv", r);
    ElMessage.success(r.msg);
  } catch (e: any) {
    ElMessage.error(e?.message ?? "stop failed");
  } finally {
    ctlBusy.value = false;
  }
}

let timer: any = null;

onMounted(async () => {
  await refreshAll();
  timer = setInterval(async () => {
    try {
      await refreshStatus();
      await refreshCtl();
    } catch {
      // 忽略定时失败，避免刷屏
    }
  }, 1000);
});

onUnmounted(() => {
  if (timer) clearInterval(timer);
});
</script>

<style scoped lang="scss">
.page {
  padding: 16px;
}
</style>
