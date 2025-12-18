type StatusRespRaw = {
  uptime_s: number
  load1: number
  mem_total_kb: number
  mem_avail_kb: number
}

export type StatusResp = {
  uptime: number
  load1: number
  memTotalKb: number
  memAvailKb: number
}

export type FileItem = {
  name: string
  size: number
  mtime: number
}

export type FileOpResp = {
  ok: boolean
  msg: string
}

async function getJson<T>(url: string): Promise<T> {
  const r = await fetch(url, { cache: 'no-store' })
  if (!r.ok) throw new Error(`${r.status} ${r.statusText}`)
  return (await r.json()) as T
}

function mapStatus(raw: StatusRespRaw): StatusResp {
  return {
    uptime: raw.uptime_s,
    load1: raw.load1,
    memTotalKb: raw.mem_total_kb,
    memAvailKb: raw.mem_avail_kb,
  }
}

export const api = {
  async status(): Promise<StatusResp> {
    const raw = await getJson<StatusRespRaw>('/cgi-bin/status.cgi')
    return mapStatus(raw)
  },
  files: () => getJson<FileItem[]>('/cgi-bin/files.cgi'),
  deleteFile: (name: string) =>
    getJson<FileOpResp>(`/cgi-bin/delete.cgi?name=${encodeURIComponent(name)}`),
  downloadUrl: (name: string) =>
    `/cgi-bin/download.cgi?name=${encodeURIComponent(name)}`,
  uploadUrl: (name: string) =>
    `/cgi-bin/upload.cgi?name=${encodeURIComponent(name)}`,
}
