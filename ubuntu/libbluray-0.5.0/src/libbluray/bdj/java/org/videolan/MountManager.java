/*
 * This file is part of libbluray
 * Copyright (C) 2010  William Hahne
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

package org.videolan;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import org.videolan.Logger;

/**
 * This class handle mounting jar files so that their contents can be accessed.
 *
 * @author William Hahne
 *
 */
public class MountManager {
    public static String mount(int jarId) throws MountException {
        String jarStr = jarIdToString(jarId);

        logger.info("Mounting JAR: " + jarStr);

        if (jarStr == null)
            throw new IllegalArgumentException();

        String oldPath = getMount(jarId);
        if (oldPath != null) {
            logger.error("JAR " + jarId + " already mounted");
            return oldPath;
        }

        String path = System.getProperty("bluray.vfs.root") + "/BDMV/JAR/" + jarStr + ".jar";

        JarFile jar = null;
        File tmpDir = null;
        try {
            jar = new JarFile(path);
            tmpDir = File.createTempFile("bdj-", "");
        } catch (IOException e) {
            e.printStackTrace();
            throw new MountException();
        }

        // create temporary directory
        tmpDir.delete();
        tmpDir.mkdir();

        try {
            byte[] buffer = new byte[32*1024];
            Enumeration entries = jar.entries();
            while (entries.hasMoreElements()) {
                JarEntry entry = (JarEntry)entries.nextElement();
                File out = new File(tmpDir + File.separator + entry.getName());

                logger.info("   mount: " + entry.getName());

                if (entry.isDirectory()) {
                    out.mkdirs();
                } else {
                    /* make sure path exists */
                    out.getParentFile().mkdirs();

                    InputStream inStream = jar.getInputStream(entry);
                    OutputStream outStream = new FileOutputStream(out);

                    int length;
                    while ((length = inStream.read(buffer)) > 0) {
                        outStream.write(buffer, 0, length);
                    }

                    inStream.close();
                    outStream.close();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            recursiveDelete(tmpDir);
            throw new MountException();
        }

        logger.info("Mounting JAR " + jarId + " complete.");

        mountPoints.put(new Integer(jarId), tmpDir);
        return tmpDir.getAbsolutePath();
    }

    public static void unmount(int jarId) {
        logger.info("Unmounting JAR: " + jarId);
        Integer id = new Integer(jarId);
        File mountPoint = (File)mountPoints.get(id);
        if (mountPoint != null) {
            recursiveDelete(mountPoint);
            mountPoints.remove(id);
        }
    }

    public static void unmountAll() {
        Iterator iterator = mountPoints.keySet().iterator();
        while (iterator.hasNext())
            unmount(((Integer)iterator.next()).intValue());
    }

    public static String getMount(int jarId) {
        Integer id = new Integer(jarId);
        if (mountPoints.containsKey(id)) {
            return ((File)mountPoints.get(id)).getAbsolutePath();
        } else {
            return null;
        }
    }

    private static String jarIdToString(int jarId) {
        if (jarId < 0 || jarId > 99999)
            return null;
        return BDJUtil.makeFiveDigitStr(jarId);
    }

    private static void recursiveDelete(File dir) {
        File[] files = dir.listFiles();
        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            if (file.isDirectory()) {
                recursiveDelete(file);
            } else {
                file.delete();
            }
        }

        dir.delete();
    }

    private static Map mountPoints = Collections.synchronizedMap(new HashMap());
    private static final Logger logger = Logger.getLogger(MountManager.class.getName());
}
