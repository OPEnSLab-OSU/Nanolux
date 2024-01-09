import {useEffect, useState} from "preact/hooks";
import {loadFromSlot, saveInSlot} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";

const Save_Entry = ({
    name,
    idx,
    handleSave,
    handleLoad
}) => {
    const {isConnected} = useConnectivity();

    const load = async () => {
        handleLoad(idx);
    }

    const save = async () => {
        handleSave(idx);
    }

    return (
        <div >
            <tr>
                <th width='150'>{name}</th>
                <th>
                    <button onClick={load}>Load Pattern</button>
                </th>
                <th>
                    <button onClick={save}>Save Pattern</button>
                </th>
                
            </tr>
        </div>
    );
}

export default Save_Entry;
