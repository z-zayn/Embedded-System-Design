<template>
  <div class="page">
    <el-row :gutter="16">
      <el-col
        :xs="24"
        :md="12"
      >
        <SystemStatusCard
          :loading="loading"
          :status="status"
          @refresh="refreshAll"
        />
      </el-col>

      <el-col
        :xs="24"
        :md="12"
      >
        <UploadCard @uploaded="refreshFiles" />
      </el-col>
    </el-row>
    <InboxFilesCard
      :loading="filesLoading"
      :files="files"
      @refresh="refreshFiles"
      @delete="deleteFile"
    />
  </div>
</template>

<script setup lang="ts">
import { api, type StatusResp, type FileItem } from "@/api";
import SystemStatusCard from "@/components/SystemStatusCard.vue";
import InboxFilesCard from "@/components/InboxFilesCard.vue";
import UploadCard from "@/components/UploadCard.vue";

const loading = ref(false);
const filesLoading = ref(false);

const status = ref<StatusResp | null>(null);
const files = ref<FileItem[]>([]);

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

async function deleteFile(name: string) {
  try {
    const r = await api.deleteFile(name)
    if (r.ok) ElMessage.success(r.msg)
    else ElMessage.error(r.msg)
    await refreshFiles()
  } catch (e: any) {
    ElMessage.error(e?.message ?? "delete failed")
  }
}

async function refreshAll() {
  loading.value = true;
  try {
    await Promise.all([refreshStatus(), refreshFiles()]);
  } catch (e: any) {
    ElMessage.error(e?.message ?? "refresh failed");
  } finally {
    loading.value = false;
  }
}

let timer: any = null;

onMounted(async () => {
  await refreshAll();
  timer = setInterval(async () => {
    try {
      await refreshStatus();
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
