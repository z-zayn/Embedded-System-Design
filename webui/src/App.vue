<template>
  <div class="page">
    <el-row :gutter="16">
      <el-col
        :xs="24"
        :md="12"
      >
        <el-card>
          <template #header>
            <div class="card-header">
              <span>System Status</span>
              <el-button
                size="small"
                :loading="loading"
                @click="refreshAll"
              >
                Refresh
              </el-button>
            </div>
          </template>

          <el-descriptions
            :column="1"
            border
          >
            <el-descriptions-item label="Uptime">
              {{ formatUptime(status?.uptime ?? 0) }}
            </el-descriptions-item>

            <el-descriptions-item label="Load (1m)">
              <el-tag :type="loadTagType(status?.load1 ?? 0)">
                {{ (status?.load1 ?? 0).toFixed(2) }}
              </el-tag>
            </el-descriptions-item>

            <el-descriptions-item label="Memory">
              <div class="mem">
                <el-progress :percentage="memPercent" />
                <div class="mem-text">
                  {{ memUsedMB }} / {{ memTotalMB }} MB
                </div>
              </div>
            </el-descriptions-item>

            <el-descriptions-item label="filesrv">
              <el-tag :type="isFilesrvRunning ? 'success' : 'danger'">
                {{ isFilesrvRunning ? "RUNNING" : "STOPPED" }}
              </el-tag>
              <span
                v-if="ctl.pid"
                class="muted"
              >PID: {{ ctl.pid }}</span>
            </el-descriptions-item>
          </el-descriptions>

          <div class="ctl">
            <el-button
              type="success"
              :disabled="isFilesrvRunning"
              :loading="ctlBusy"
              @click="startSrv"
            >
              Start
            </el-button>
            <el-button
              type="danger"
              :disabled="!isFilesrvRunning"
              :loading="ctlBusy"
              @click="stopSrv"
            >
              Stop
            </el-button>
          </div>
        </el-card>
      </el-col>

      <el-col
        :xs="24"
        :md="12"
      >
        <el-card>
          <template #header>
            <div class="card-header">
              <span>Files in /data/inbox</span>
              <el-button
                size="small"
                :loading="filesLoading"
                @click="refreshFiles"
              >
                Refresh
              </el-button>
            </div>
          </template>

          <el-table
            :data="files"
            height="360"
            size="small"
            stripe
          >
            <el-table-column
              prop="name"
              label="Name"
              min-width="160"
            />
            <el-table-column
              label="Size"
              width="120"
            >
              <template #default="scope">
                {{
                  formatSize(scope.row.size)
                }}
              </template>
            </el-table-column>
            <el-table-column
              label="Modified"
              width="180"
            >
              <template #default="scope">
                {{
                  formatTime(scope.row.mtime)
                }}
              </template>
            </el-table-column>
          </el-table>

          <div class="hint">
            Tip: Stop filesrv and run filecli again to show “upload fails when
            stopped” in your demo.
          </div>
        </el-card>
      </el-col>
    </el-row>

    <el-card style="margin-top: 16px">
      <template #header>
        <span>Last Operation</span>
      </template>
      <el-alert
        v-if="lastMsg"
        :title="lastMsg"
        type="info"
        show-icon
      />
      <pre class="pre">{{ lastRaw }}</pre>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from "vue";
import { ElMessage } from "element-plus";
import { api, type StatusResp, type FileItem, type CtlResp } from "./api";

const loading = ref(false);
const filesLoading = ref(false);
const ctlBusy = ref(false);

const status = ref<StatusResp | null>(null);
const files = ref<FileItem[]>([]);
const ctl = ref<CtlResp>({ ok: true, msg: "init", running: false });

const lastMsg = ref("");
const lastRaw = ref("");

const isFilesrvRunning = computed(() => !!ctl.value.running);

const memPercent = computed(() => {
  const t = status.value?.memTotalKb ?? 0;
  const a = status.value?.memAvailKb ?? 0;
  if (t <= 0 || a < 0) return 0;
  const used = t - a;
  return Math.max(0, Math.min(100, Math.round((used / t) * 100)));
});

const memTotalMB = computed(() =>
  Math.round((status.value?.memTotalKb ?? 0) / 1024)
);
const memUsedMB = computed(() => {
  const t = status.value?.memTotalKb ?? 0;
  const a = status.value?.memAvailKb ?? 0;
  if (t <= 0 || a < 0) return 0;
  return Math.round((t - a) / 1024);
});

function formatUptime(sec: number) {
  sec = Math.max(0, Math.floor(sec));
  const d = Math.floor(sec / 86400);
  sec %= 86400;
  const h = Math.floor(sec / 3600);
  sec %= 3600;
  const m = Math.floor(sec / 60);
  sec %= 60;
  const parts = [];
  if (d) parts.push(`${d}d`);
  if (h) parts.push(`${h}h`);
  if (m) parts.push(`${m}m`);
  parts.push(`${sec}s`);
  return parts.join(" ");
}

function loadTagType(load1: number) {
  if (load1 < 0.8) return "success";
  if (load1 < 1.5) return "warning";
  return "danger";
}

function formatSize(n: number) {
  if (n < 1024) return `${n} B`;
  if (n < 1024 * 1024) return `${(n / 1024).toFixed(1)} KB`;
  return `${(n / (1024 * 1024)).toFixed(1)} MB`;
}

function formatTime(epochSec: number) {
  const d = new Date(epochSec * 1000);
  return d.toLocaleString();
}

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

.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.mem {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.mem-text {
  color: #666;
  font-size: 12px;
}

.muted {
  margin-left: 10px;
  color: #666;
}

.ctl {
  margin-top: 12px;
  display: flex;
  gap: 10px;
}

.pre {
  background: #0b1020;
  color: #d6e1ff;
  padding: 12px;
  border-radius: 6px;
  overflow: auto;
}

.hint {
  margin-top: 10px;
  color: #666;
  font-size: 12px;
}
</style>
