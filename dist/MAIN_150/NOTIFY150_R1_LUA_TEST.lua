-- NOTIFY150_R1 only. Defines helpers; no request is issued on load.
-- Follow NOTIFY150_R1_TEST_2026-11-06.md. Do not use on older ASIs.
DSF150Baseline = { ids = {
  62, 118, 119, 123, 124, 125, 126, 127, 128, 129, 130, 131,
  132, 133, 134, 135, 136, 138, 139, 142, 143, 144, 146, 147,
  148, 149, 150, 151, 152, 153, 154, 156, 158, 159, 160, 161,
  162, 163, 164, 165, 167, 170, 171, 172, 173, 175, 176, 178,
  179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
  191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
  203, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 216,
  217, 221, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
  234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 246, 248,
  249, 250, 251, 252, 255, 257, 258, 259, 265, 266, 267, 268,
  269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280,
  281, 282, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293,
  298, 299, 300, 301, 302, 303
} }

function DSF150Baseline.request()
  assert(#DSF150Baseline.ids == 150, "Expected exactly 150 IDs")
  if TrafficSpooler.Busy() then
    print("Spooler busy. Let the transition finish before retrying.")
    return false
  end
  TrafficSpooler.RequestTrafficVehicles(#DSF150Baseline.ids, DSF150Baseline.ids)
  print("Submitted 150-model roster. Acceptance is not load completion.")
  return true
end

function DSF150Baseline.status()
  local busy = TrafficSpooler.Busy()
  local loaded = TrafficSpooler.AllTrafficVehiclesLoaded()
  print("DSF150 status: busy=" .. tostring(busy) .. " allTrafficLoaded=" .. tostring(loaded))
  return loaded, busy
end
