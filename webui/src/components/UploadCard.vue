<template>
  <el-card class="upload-card">
    <template #header>
      <div class="card-header">
        <span>文件上传</span>
      </div>
    </template>

    <el-upload
      drag
      :multiple="false"
      :auto-upload="true"
      :http-request="doUpload"
    >
      <div class="el-upload__text">
        Drop file here or <em>click to upload</em>
      </div>
    </el-upload>
  </el-card>
</template>

<script setup lang="ts">
import { type UploadRequestOptions, } from "element-plus";
import { api } from "@/api";

const emit = defineEmits<{
  (e: "uploaded"): void;
}>();

function doUpload(options: UploadRequestOptions) {
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
.upload-card {
  height: 100%;
}
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
