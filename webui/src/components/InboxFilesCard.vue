<template>
  <el-card>
    <template #header>
      <div class="card-header">
        <span>Files in /data/inbox</span>
        <el-button
          size="small"
          :loading="loading"
          @click="$emit('refresh')"
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
          {{ formatSize(scope.row.size) }}
        </template>
      </el-table-column>
      <el-table-column
        label="Modified"
        width="180"
      >
        <template #default="scope">
          {{ formatTime(scope.row.mtime) }}
        </template>
      </el-table-column>
    </el-table>

    <div class="hint">
      Tip: Stop filesrv and run filecli again to show “upload fails when stopped”
      in your demo.
    </div>
  </el-card>
</template>

<script setup lang="ts">
import type { FileItem } from "../api";

defineProps<{
  loading: boolean;
  files: FileItem[];
}>();

defineEmits<{
  (e: "refresh"): void;
}>();

function formatSize(n: number) {
  if (n < 1024) return `${n} B`;
  if (n < 1024 * 1024) return `${(n / 1024).toFixed(1)} KB`;
  return `${(n / (1024 * 1024)).toFixed(1)} MB`;
}

function formatTime(epochSec: number) {
  const d = new Date(epochSec * 1000);
  return d.toLocaleString();
}
</script>

<style scoped lang="scss">
.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.hint {
  margin-top: 10px;
  color: #666;
  font-size: 12px;
}
</style>
