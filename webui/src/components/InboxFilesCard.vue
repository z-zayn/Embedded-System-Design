<template>
  <el-card style="margin-top: 16px">
    <template #header>
      <div class="card-header">
        <span>文件列表</span>
        <el-button
          size="small"
          :loading="loading"
          @click="$emit('refresh')"
        >
          刷新
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
      <el-table-column
        label="Actions"
        width="160"
        fixed="right"
      >
        <template #default="scope">
          <el-button
            size="small"
            type="primary"
            link
            @click="onDownload(scope.row.name)"
          >
            Download
          </el-button>

          <el-button
            size="small"
            type="danger"
            link
            @click="onDelete(scope.row.name)"
          >
            Delete
          </el-button>
        </template>
      </el-table-column>
    </el-table>
  </el-card>
</template>

<script setup lang="ts">
import type { FileItem } from "@/api";

defineProps<{
  loading: boolean;
  files: FileItem[];
}>();

const emit = defineEmits<{
  (e: "refresh"): void;
  (e: "delete", name: string): void;
}>();

function downloadUrl(name: string) {
  return `/cgi-bin/download.cgi?name=${encodeURIComponent(name)}`;
}

function onDownload(name: string) {
  const url = downloadUrl(name)
  // eslint-disable-next-line no-undef
  const a = document.createElement('a')
  a.href = url
  // eslint-disable-next-line no-undef
  document.body.appendChild(a)
  a.click()
  a.remove()
}

async function onDelete(name: string) {
  await ElMessageBox.confirm(`Delete "${name}" from /data/inbox?`, "Confirm", {
    type: "warning",
  });
  emit("delete", name);
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
</script>

<style scoped lang="scss">
.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}
</style>
