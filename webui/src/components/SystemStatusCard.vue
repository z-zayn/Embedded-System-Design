<template>
  <el-card>
    <template #header>
      <div class="card-header">
        <span>系统状态</span>
        <el-button
          size="small"
          :loading="loading"
          @click="$emit('refresh')"
        >
          刷新
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
        <el-tag :type="running ? 'success' : 'danger'">
          {{ running ? "RUNNING" : "STOPPED" }}
        </el-tag>
        <span
          v-if="pid"
          class="muted"
        >PID: {{ pid }}</span>
      </el-descriptions-item>
    </el-descriptions>

    <div class="ctl">
      <el-button
        type="success"
        :disabled="running"
        :loading="ctlBusy"
        @click="$emit('start')"
      >
        Start
      </el-button>
      <el-button
        type="danger"
        :disabled="!running"
        :loading="ctlBusy"
        @click="$emit('stop')"
      >
        Stop
      </el-button>
    </div>
  </el-card>
</template>

<script setup lang="ts">
import type { StatusResp } from "../api";

const props = defineProps<{
  loading: boolean;
  ctlBusy: boolean;
  status: StatusResp | null;
  running: boolean;
  pid?: number;
}>();

defineEmits<{
  (e: "refresh"): void;
  (e: "start"): void;
  (e: "stop"): void;
}>();

const memPercent = computed(() => {
  const t = props.status?.memTotalKb ?? 0;
  const a = props.status?.memAvailKb ?? 0;
  if (t <= 0 || a < 0) return 0;
  const used = t - a;
  return Math.max(0, Math.min(100, Math.round((used / t) * 100)));
});

const memTotalMB = computed(() =>
  Math.round((props.status?.memTotalKb ?? 0) / 1024)
);

const memUsedMB = computed(() => {
  const t = props.status?.memTotalKb ?? 0;
  const a = props.status?.memAvailKb ?? 0;
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
  const parts: string[] = [];
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
</script>

<style scoped lang="scss">
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
</style>
