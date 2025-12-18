<template>
  <el-card>
    <template #header>
      <div class="card-header">
        <span>Upload to /data/inbox</span>
      </div>
    </template>

    <el-upload
      drag
      :show-file-list="true"
      :limit="1"
      :multiple="false"
      :auto-upload="true"
      :http-request="doUpload"
    >
      <div class="el-upload__text">
        Drop file here or <em>click to upload</em>
      </div>
      <template #tip>
        <div class="tip">
          Files will appear in the Inbox list after upload completes.
        </div>
      </template>
    </el-upload>
  </el-card>
</template>

<script setup lang="ts">
import { api } from "@/api";

const emit = defineEmits<{
  (e: "uploaded"): void;
}>();

// Element Plus Upload 自定义请求：options 结构里包含 file/onProgress/onSuccess/onError
function doUpload(options: any) {
  const file: File = options.file;
  const url = api.uploadUrl(file.name);

  const xhr = new XMLHttpRequest();
  xhr.open("POST", url, true);
  xhr.setRequestHeader("Content-Type", "application/octet-stream");

  xhr.upload.onprogress = (e) => {
    if (e.lengthComputable && options.onProgress) {
      options.onProgress({ percent: (e.loaded / e.total) * 100 });
    }
  };

  xhr.onload = () => {
    try {
      const ok = xhr.status >= 200 && xhr.status < 300;
      const resp = xhr.responseText ? JSON.parse(xhr.responseText) : { ok };
      if (ok && resp.ok !== false) {
        options.onSuccess?.(resp, file);
        ElMessage.success(resp.msg ?? "uploaded");
        emit("uploaded");
      } else {
        const msg = resp.msg ?? `upload failed (${xhr.status})`;
        options.onError?.(new Error(msg));
        ElMessage.error(msg);
      }
    } catch (e: any) {
      options.onError?.(e);
      ElMessage.error(e?.message ?? "upload parse failed");
    }
  };

  xhr.onerror = () => {
    options.onError?.(new Error("network error"));
    ElMessage.error("network error");
  };

  xhr.send(file);
}
</script>

<style scoped lang="scss">
.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.tip {
  color: #666;
  font-size: 12px;
  margin-top: 6px;
}
</style>
