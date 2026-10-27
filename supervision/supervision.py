#!/usr/bin/env python3
"""Supervision série de la ligne de production (Version A).
 
Lit les lignes EVT/STAT envoyées par la carte, affiche un tableau de bord texte,
enregistre les événements et le statut dans des fichiers CSV horodatés, puis affiche un résumé.
 
Utilisation:
  Mode réel  : pip install pyserial ; python supervision.py --port COM3        (Windows)
                                      python supervision.py --port /dev/ttyACM0 (Linux/Mac)
  Mode rejeu : python supervision.py --replay journal.log   (relit un journal déjà enregistré, sans carte)
Commandes envoyées à la carte (mode réel): taper s, x, r, f, i ou h puis Entrée.
"""
import argparse, csv, sys, threading, time
from datetime import datetime
 
def parse(line):
    """Retourne ('EVT'|'STAT', champs) ou None si la ligne n'est pas exploitable (commentaire, bruit...)."""
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    parts = line.split(",")
    if parts[0] == "EVT" and len(parts) >= 3:
        return "EVT", parts[1:]
    if parts[0] == "STAT" and len(parts) == 10:
        return "STAT", parts[1:]
    return None
 
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port"); ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--replay"); ap.add_argument("--out", default=".")
    a = ap.parse_args()
    if not a.port and not a.replay:
        ap.error("indiquer --port (carte connectée) ou --replay (fichier journal)")
 
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    fe = open(f"{a.out}/evenements_{stamp}.csv", "w", newline="", encoding="utf-8")
    fs = open(f"{a.out}/statuts_{stamp}.csv", "w", newline="", encoding="utf-8")
    we, ws = csv.writer(fe), csv.writer(fs)
    we.writerow(["ms", "type", "detail1", "detail2", "detail3", "detail4", "detail5"])
    ws.writerow(["ms", "etat", "total", "conformes", "non_conformes", "defauts", "defaut_actif", "distance_mm", "qualite"])
 
    if a.replay:
        source = open(a.replay, encoding="utf-8", errors="replace")
        ser = None
    else:
        import serial  # pyserial
        ser = serial.Serial(a.port, a.baud, timeout=0.2)
        source = None
        def send():
            for l in sys.stdin:
                ser.write(l.strip()[:1].encode())
        threading.Thread(target=send, daemon=True).start()
 
    last = None; nfaults = 0; nbad = 0; npieces = 0
    try:
        while True:
            raw = source.readline() if source else ser.readline().decode("utf-8", "replace")
            if source and raw == "":
                break
            if not raw:
                continue
            p = parse(raw)
            if p is None:
                if raw.strip() and not raw.startswith("#"): nbad += 1
                continue
            kind, f = p
            if kind == "EVT":
                we.writerow(f[:7] + [""] * (7 - len(f[:7])))
                print(f"[{int(f[0])/1000:8.3f}s] {','.join(f[1:])}")
                if f[1] == "DEFAUT": nfaults += 1
                if f[1] == "PIECE_TRIEE": npieces += 1
            else:
                ws.writerow(f); last = f
                print(f"   état={f[1]:<10} total={f[2]} conformes={f[3]} non_conformes={f[4]} défauts={f[5]} actif={f[6]} d={f[7]}mm")
    except KeyboardInterrupt:
        pass
    finally:
        fe.close(); fs.close()
    print("\n=== Résumé ===")
    print(f"pièces triées (événements): {npieces} | défauts (événements): {nfaults} | lignes ignorées: {nbad}")
    if last:
        total, ok, nok = int(last[2]), int(last[3]), int(last[4])
        print(f"dernier statut: total={total} conformes={ok} non_conformes={nok} -> "
              f"{'COHÉRENT' if total == ok + nok else 'INCOHÉRENT (total != conformes + non conformes)'}")
    print(f"fichiers: evenements_{stamp}.csv, statuts_{stamp}.csv")
 
if __name__ == "__main__":
    main()
 
